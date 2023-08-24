

/*
NAvestock Scoreboard Controler
 by dlf (Metodo2 srl)

 last modified 17 August 2023
 by Lefras Coetzee

 */
#include "SevenSegmentDisplayController.h"
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Firebase_Arduino_WiFiNINA.h>
#include "arduino_secrets.h"


/*
 * WiFi Setup
 * sensitive data in the Secret tab/arduino_secrets.h
*/
const char ssid[] = SECRET_SSID;    // your network SSID (name)
const char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;             // your network key Index number
int wifi_status = WL_IDLE_STATUS;  // the Wifi radio's status


/*
 * SevenSegmentDisplayController.h library to control the binary code sent to the shift register and 7Segment displays
*/
SevenSegmentDisplayController sevenSegController;


/* ------------------------
 * Firebase Setup
 * Define:
 *  - Firebase data object
 *  - Firebase path to monitor
  * sensitive data in the Secret tab/arduino_secrets.h
  -------------------------- */
String firebaseDB_URL = DATABASE_URL;
String firebaseDB_SecretKey =  DATABASE_SECRET;
StaticJsonDocument<256> ScoreboardDoc;
FirebaseData firebaseDataStream;
const String path = "/Navestock_Scoreboard";

/* 
 * Arduino Setup
 */
void setup() {

 //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
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
  Serial.println("Starting connection to Firebase ...");
  Serial.println();
  
  Firebase.begin(firebaseDB_URL, firebaseDB_SecretKey, ssid, pass);
  Firebase.reconnectWiFi(false);

Serial.println("Starting connection to Firebase stream ...");
  if (!Firebase.beginStream(firebaseDataStream, path))
  {
    Serial.print("Can't connect stream, ");
    Serial.print(firebaseDataStream.errorReason());
    Serial.println();
  } else {
    Serial.print("Connected to Firebase Stream: , ");
    Serial.print( path);
    Serial.println();
  }

}

/* 
 * Arduino Loop
 */
void loop() {
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
            auto Batsman1 = ScoreboardDoc["Batsman1"].as<String>();
            auto Batsman2 = ScoreboardDoc["Batsman2"].as<String>();
            auto Overs = ScoreboardDoc["Overs"].as<String>();
            auto Teamscore = ScoreboardDoc["Teamscore"].as<String>();
           
            String scorboardDataTop = String(sevenSegController.displayUnitFormater(3, Batsman1) + sevenSegController.displayUnitFormater(3, Teamscore) + sevenSegController.displayUnitFormater(3, Batsman2));

            //Call function to Write data to the display
            Serial.println("Scoreboard Data: " + scorboardDataTop);
          }
    }
  
  }

/*
 **************************************************************************************
 * Connect to the WIFI Network
 * IP Address & MAC Address
 **************************************************************************************
*/
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

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void checkWiFiStatus() {
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "shield");
  } else if (WiFi.status() == WL_IDLE_STATUS) {
    Serial.println("WiFi idle");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "idle");
  } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
    Serial.println("No WiFi networks available");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "unavailable");
  } else if (WiFi.status() == WL_SCAN_COMPLETED) {
    Serial.println("WiFi scan completed");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "scan");
  } else if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected to WiFi network: ");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "connected");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else if (WiFi.status() == WL_CONNECT_FAILED) {
    Serial.println("Connection failed");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "failed");
  } else if (WiFi.status() == WL_CONNECTION_LOST) {
    Serial.println("Connection lost");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "lost");
  } else if (WiFi.status() == WL_DISCONNECTED) {
    Serial.println("Disconnected from WiFi");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "disconnected");
  } else if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "no#module");
  } else {
    Serial.println("Communication with WiFi unknow error!");
    writeDataToDisplay(0, "wifi");
    writeDataToDisplay(1, "error");
  }  
}


/*
 **************************************************************************************
 * Write input in "dataToDisplay" to the display board
 **************************************************************************************
*/
void writeDataToDisplay(int ledSegment, String dataToDisplay){

  //Segment Config > {SH_CP/SRCK/clockPin (Yellow), DS/SER/DataPin (White), ST_CP/RCK/LatchPin (Green), 7Segment LED's}
  const int myLatchPin = 2;     // ST_CP/RCK/LatchPin (Green)
  const int myDataPin =  3;     // DS/SER/DataPin (White)
  const int myClockPin = 4;     // SH_CP/SRCK/clockPin (Yellow)
  const int mySegmentLenth = 2; // 7Segment LED's


  // Write data to the display
  dataToDisplay.trim();
  int dataToDisplayLength = dataToDisplay.length();

  // Ensure the dataToDisplay length matches the mySegmentLength
  if (dataToDisplayLength < mySegmentLenth) {
    while (dataToDisplay.length() < mySegmentLenth) {
      dataToDisplay = "#" + dataToDisplay;
    }
  } else if (dataToDisplayLength > mySegmentLenth) {
    dataToDisplay = dataToDisplay.substring(0, mySegmentLenth);
  }

  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(myLatchPin, LOW);

  Serial.print("dataToDisplay: ");
  Serial.println(dataToDisplay);

  //Write data to shift register
  for (int i = mySegmentLenth - 1; i >= 0; i--)
  {
    String tempChar = String(dataToDisplay.charAt(i));
    shiftOut(myDataPin, myClockPin, LSBFIRST, sevenSegController.binaryToDisplay(tempChar));
  }
  //return the latch pin high to signal chip that it
  //no longer needs to listen for information
  digitalWrite(myLatchPin, HIGH);
 

}