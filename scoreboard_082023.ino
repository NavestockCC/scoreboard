

/****************************************************************************************************
Navestock Scoreboard Controller
 last modified 19 August 2023
 by Lefras Coetzee


Connection to Arduino output pins
 {2 -> SH_CP/SRCK/clockPin (Yellow), 3 - >DS/SER/DataPin (White), 4 -> ST_CP/RCK/LatchPin (Green)}
 {5 -> SH_CP/SRCK/clockPin (Yellow), 6 - >DS/SER/DataPin (White), 7 -> ST_CP/RCK/LatchPin (Green)}

 ****************************************************************************************************/

#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Firebase_Arduino_WiFiNINA.h>
#include "arduino_secrets.h"
#include "SevenSegmentDisplayController.h"

/*-------------------------------------------------
 * WiFi Setup
 * sensitive data in the Secret arduino_secrets.h
  -------------------------------------------------*/
const char ssid[] = SECRET_SSID;    // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;             // your network key Index number
int wifi_status = WL_IDLE_STATUS;  // the Wifi radio's status


/*----------------------------------------------------------------------------
 * SevenSegmentDisplayController.h library to control the 7Segment displays
 *----------------------------------------------------------------------------*/
SevenSegmentDisplayController sevenSegController;


/* ------------------------
 * Firebase Setup
 * Define:
 *  - Firebase data object
 *  - Firebase path to monitor
  * sensitive data in the Secret arduino_secrets.h
  -------------------------- */
const String firebaseDB_URL = DATABASE_URL;
const String path = DATABASE_PATH;
const String firebaseDB_SecretKey =  DATABASE_SECRET;
StaticJsonDocument<256> ScoreboardDoc;
FirebaseData firebaseDataStream;


/* 
 * Arduino Setup
 */
void setup() {
/* ------------------------------------------------------------------
  Initialize serial and wait for port to open:
  wait for serial port to connect. Needed for native USB port only
  ------------------------------------------------------------------- */
  Serial.begin(9600);
  while (!Serial) { 
  }

  /* --------------------------------
  Initialise the arduino OUTPUT pins
  ----------------------------------- */
  sevenSegController.initialiseArduinoPins();

  /* ------------------------
         Connect to Wifi
   -------------------------- */
    connectToWiFi(ssid, pass);
 
  /* ------------------------
      Connect to Firebase
   -------------------------- */
  Firebase.begin(firebaseDB_URL, firebaseDB_SecretKey, ssid, pass);
  Firebase.reconnectWiFi(false);
  if (!Firebase.beginStream(firebaseDataStream, path))
  {
    Serial.print("Can't connect Firebase stream, ");
    Serial.print(firebaseDataStream.errorReason());
    Serial.println();
  } else {
    Serial.print("Connected to Firebase Stream: , ");
    Serial.print( path);
    Serial.println();
  }
}// end setup


/* 
 * Arduino Loop
 */
void loop() {
   /* -------------------------------------------
     Wifi connection monitoring
     reconnect if not connected
   -------------------------------------------- */
    if (!isWiFiConnected()) {
      Serial.println("WiFi connection lost. Reconnecting...");
      connectToWiFi(ssid, pass);
  }


 /* -------------------------------------------
     Firebase handle data stream monitoring
   -------------------------------------------- */
  if (!Firebase.readStream(firebaseDataStream))
  {
    Serial.println("Can't read stream, "+ firebaseDataStream.errorReason());
  }

  if (firebaseDataStream.streamTimeout())
  {
    Serial.println("Stream timed out, resuming...");
  }

  // streamAvailable returnes true when stream data was received in buffer
  if (firebaseDataStream.streamAvailable())
  {
    Serial.println("stream available....");
   //Get JSON data from the Firebase RTDB
     Firebase.getJSON(firebaseDataStream, path);
   // Parse JSON response with ArduinoJSON
     DeserializationError err = deserializeJson(ScoreboardDoc, firebaseDataStream.jsonData());
      if (err) {
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(err.c_str());
          } else {
            //Format data to display
            String scoreboardDataTop = String(sevenSegController.displayUnitFormater(ScoreboardDoc["Batsman1"].as<String>(), "Top", "Batsman1") + sevenSegController.displayUnitFormater(ScoreboardDoc["Teamscore"].as<String>(), "Top", "Teamscore") + sevenSegController.displayUnitFormater(ScoreboardDoc["Batsman2"].as<String>(), "Top", "Batsman2"));
            String scoreboardDataBottom = String(sevenSegController.displayUnitFormater(ScoreboardDoc["Overs"].as<String>(), "Bottom", "Overs") + sevenSegController.displayUnitFormater(ScoreboardDoc["Wickets"].as<String>(), "Bottom", "Wickets") + sevenSegController.displayUnitFormater(ScoreboardDoc["Target"].as<String>(), "Bottom", "Target"));
            //Write date to the ShiftRegisters
            sevenSegController.writeDataToDisplay(scoreboardDataTop, "Top");
            sevenSegController.writeDataToDisplay(scoreboardDataBottom, "Bottom");

          }
    }
  } // end loop

/*********************************
 * Connect to the WIFI Network.
 *  Print IP Address & MAC Address to serial monitor
 * @param network SSID
 * @param network password
 **********************************/
void connectToWiFi(const char* my_ssid, const char* my_pass ) {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    checkWiFiStatus();
    wifi_status = WiFi.begin(my_ssid, my_pass);

    // Wait for WiFi connection or timeout
    int timeout = 30; // seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      checkWiFiStatus();
      delay(1000);
      timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connected to WiFi!");
      checkWiFiStatus();
    } else {
      Serial.println("WiFi connection failed.");
      checkWiFiStatus();
    }
  }
}

/*********************************
 * Check WIFI Network status
 * @return bool - True if status WL_CONNECTED
 **********************************/
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

/*********************************
 * Check WIFI Network status
 * Print to serial monitor the detailed connection status
 * Send network status to scoreboard 
 **********************************/
void checkWiFiStatus() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("shield", "Bottom");
  } else if (WiFi.status() == WL_IDLE_STATUS) {
    Serial.println("WiFi idle");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("idle", "Bottom");
  } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
    Serial.println("No WiFi networks available");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("unavailable", "Bottom");
    Serial.println("WiFi scan completed");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("scan", "Bottom");
  } else if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WiFi network: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("connected", "Bottom");    
  } else if (WiFi.status() == WL_CONNECT_FAILED) {
    Serial.println("Connection failed");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("failed", "Bottom"); 
  } else if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("Connection lost");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("lost", "Bottom");
  } else if (WiFi.status() == WL_DISCONNECTED) {
    Serial.println("Disconnected from WiFi");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("disconnected", "Bottom");
  } else if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("module", "Bottom");
  } else {
    Serial.println("Communication with WiFi unknow error!");
    sevenSegController.writeDataToDisplay("wifi", "Top");
    sevenSegController.writeDataToDisplay("error", "Bottom");
  }  
}
