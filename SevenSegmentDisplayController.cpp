#include "SevenSegmentDisplayController.h"


// Define constants for JSON strings
// Alpha Numeric display HEX definitions
const char* DISPLAY_JSON = "{\"0\": \"0xFC\",\"1\": \"0x60\",\"2\": \"0xDA\",\"3\": \"0xF2\",\"4\": \"0x66\",\"5\": \"0xB6\",\"6\": \"0xBE\",\"7\": \"0xE0\",\"8\": \"0xFE\",\"9\": \"0xE6\",\"a\": \"0xFA\",\"b\": \"0x3E\",\"c\": \"0x1A\",\"d\": \"0x7A\",\"e\": \"0x9E\",\"f\": \"0x8E\",\"g\": \"0xBC\",\"h\": \"0x2E\",\"i\": \"0xA0\",\"j\": \"0xB0\",\"k\": \"0xAE\",\"l\": \"0x1C\",\"m\": \"0xAA\",\"n\": \"0x2A\",\"o\": \"0x3A\",\"p\": \"0xCE\",\"q\":\"0xE6\",\"r\": \"0xA\",\"s\": \"0xB4\",\"t\": \"0x1E\",\"u\": \"0x38\",\"v\": \"0x54\",\"w\": \"0x56\",\"x\": \"0x28\",\"y\": \"0x76\",\"z\": \"0xD8\",\"*\": \"0x1\",\"#\": \"0x0\"}";

//Scoreboard config
const char* SCOREBOARD_CONFIG_JSON = "{\"displaySegments\":[{\"arduinoControlPins\":{\"clockPin\":2,\"dataPin\":3,\"latchPin\":4},\"displaySegmentsName\":\"Top\",\"displaySegmentLength\":2,\"displayUnitsNumberOf\":3,\"displayUnits\":[{\"displayUnitName\":\"Batsman1\",\"displayUnitLength\":3,\"displayUnitPosition\":0},{\"displayUnitName\":\"Teamscore\",\"displayUnitLength\":3,\"displayUnitPosition\":1},{\"displayUnitName\":\"Batsman2\",\"displayUnitLength\":3,\"displayUnitPosition\":2}]},{\"arduinoControlPins\":{\"clockPin\":5,\"dataPin\":6,\"latchPin\":7},\"displaySegmentsName\":\"Bottom\",\"displaySegmentLength\":2,\"displayUnitsNumberOf\":3,\"displayUnits\":[{\"displayUnitName\":\"Overs\",\"displayUnitLength\":2,\"displayUnitPosition\":0},{\"displayUnitName\":\"Wickets\",\"displayUnitLength\":1,\"displayUnitPosition\":1},{\"displayUnitName\":\"Target\",\"displayUnitLength\":3,\"displayUnitPosition\":2}]}],\"displaySegmentsNumberOf\":2}";


SevenSegmentDisplayController::SevenSegmentDisplayController() {
  // Deserialize the JSON document in the constructor
  DeserializationError error_alphabet = deserializeJson(display_doc, DISPLAY_JSON);
    if (error_alphabet) {
    Serial.print(F("deserialize scoreboardDisplayConfig_json failed: "));
    Serial.println(error_alphabet.f_str());
    return;
  }
  DeserializationError error_display = deserializeJson(scoreboardDisplayConfig_doc, SCOREBOARD_CONFIG_JSON);
    if (error_display) {
    Serial.print(F("deserialize scoreboardDisplayConfig_json failed: "));
    Serial.println(error_display.f_str());
    return;
  }
  arduinoPinConfig_doc = scoreboardDisplayConfig_doc.as<JsonObject>();

}

 /***************************************************************************************
 * Initialise the Arduino Output Pins
 *  Takes setup data defined in the SCOREBOARD_CONFIG_JSON
 ****************************************************************************************/
void SevenSegmentDisplayController::initialiseArduinoPins(){

    for (int i = 0; i < arduinoPinConfig_doc["displaySegmentsNumberOf"]; i++) {
      pinMode(arduinoPinConfig_doc["displaySegments"][i]["arduinoControlPins"]["clockPin"], OUTPUT);
      pinMode(arduinoPinConfig_doc["displaySegments"][i]["arduinoControlPins"]["dataPin"], OUTPUT);
      pinMode(arduinoPinConfig_doc["displaySegments"][i]["arduinoControlPins"]["latchPin"], OUTPUT);
    }
}

/*
 **************************************************************************************
 * Returns a Hex Decimal value of the input character 
 * based on the json definition of the alpha numeric DISPLAY_JSON doc
 * @return - byte - hex-decimal value of the input character
 ***************************************************************************************/
byte SevenSegmentDisplayController::binaryToDisplay(String toDisplay) {
  String byteString = display_doc[toDisplay];
  byte resultByte = (byte)strtol(byteString.c_str(), NULL, 16);
  return resultByte;
}

/***************************************************************************************
 * Format the dataToDisplay to the correct display unit length.
 * @param displayUnitLength - the number of 7SegmentDisplays in the unit e.g. Bastman 1 has 3 7Segment Displays
 * @param dataToDisplay - the data string to display
 * @return String - of a length == to the displayUnitLength. 
 *         If dataToDisplay was to long return digits from the left. 
 *         If dataToDisplay was to short fill from the left with #
 ***************************************************************************************/
String SevenSegmentDisplayController::displayUnitFormater(String dataToDisplay, const char* segmentName, const char* unitName){

  // Write data to the display
  dataToDisplay.trim();
  int dataToDisplayLength = dataToDisplay.length();
  int displayUnitLength = getDisplayUnitLength(segmentName, unitName);


  // Ensure the dataToDisplay length matches the mySegmentLength
  if (dataToDisplayLength < displayUnitLength) {
    while (dataToDisplay.length() < displayUnitLength) {
      dataToDisplay = "#" + dataToDisplay;
    }
  } else if (dataToDisplayLength > displayUnitLength) {
    dataToDisplay = dataToDisplay.substring(0, displayUnitLength);
  }

  return dataToDisplay;
}

/***************************************************************************************
 * Format the dataToDisplay to the correct Segment length.
 * @param displayUnitLength - the number of 7SegmentDisplays in the segment e.g. Top/Bottom
 * @param dataToDisplay - the data string to display
 * @return String - of a length == to the displaySegmentLength. 
 *         If dataToDisplay was to long return digits from the left. 
 *         If dataToDisplay was to short fill from the left with #
 ***************************************************************************************/
String SevenSegmentDisplayController::displaySegmentFormater(String dataToDisplay, const char* segmentName){

  // Write data to the display
  dataToDisplay.trim();
  int dataToDisplayLength = dataToDisplay.length();
  int displaySegmentLength = getDisplaySegmentLength(segmentName);


  // Ensure the dataToDisplay length matches the mySegmentLength
  if (dataToDisplayLength < displaySegmentLength) {
    while (dataToDisplay.length() < displaySegmentLength) {
      dataToDisplay = "#" + dataToDisplay;
    }
  } else if (dataToDisplayLength > displaySegmentLength) {
    dataToDisplay = dataToDisplay.substring(0, displaySegmentLength);
  }

  return dataToDisplay;
}

 /***************************************************************************************
 * get the number of 7Segment Displays for a UNIT
 * Takes setup data defined in the SCOREBOARD_CONFIG_JSON
 * @param char* segmentName - name of the scoreboard segment e.g Top/Bottom
 * @param char* unitName - name of the unit within the segment e.g. BatA/Total/Wickets
 * @return int -  length of the display unit. 
 ****************************************************************************************/
int SevenSegmentDisplayController::getDisplayUnitLength(const char* segmentName, const char* unitName) {
  // Check if the displaySegments array exists
  if (arduinoPinConfig_doc.containsKey("displaySegments")) {
    JsonArray segments = arduinoPinConfig_doc["displaySegments"];

    // Iterate through the segments
    for (JsonObject segment : segments) {
      // Check if the "displaySegmentsName" matches the specified segmentName
      if (segment.containsKey("displaySegmentsName") && segment["displaySegmentsName"].as<String>() == segmentName) {
        // Check if the displayUnits array exists within the segment
        if (segment.containsKey("displayUnits")) {
          JsonArray units = segment["displayUnits"];

          // Iterate through the units
          for (JsonObject unit : units) {
            // Check if the "displayUnitName" matches the specified unitName
            if (unit.containsKey("displayUnitName") && unit["displayUnitName"].as<String>() == unitName) {
              // Retrieve and return the "displayUnitLength" as a int
              return unit["displayUnitLength"].as<int>();
            }
          }
        }
      }
    }
  }

  // Return 0 if the requested data is not found
  return 0;
}


 /***************************************************************************************
 * get the length of the display SEGMENT 
 * Takes setup data defined in the SCOREBOARD_CONFIG_JSON
 * @param char* segmentName - name of the scoreboard segment e.g Top/Bottom
 * @return int -  length of the display SEGMENT. 
 ****************************************************************************************/
int SevenSegmentDisplayController::getDisplaySegmentLength(const char* segmentName) {
  // Check if the displaySegments array exists
  if (arduinoPinConfig_doc.containsKey("displaySegments")) {
    JsonArray segments = arduinoPinConfig_doc["displaySegments"];

    // Iterate through the segments
    for (JsonObject segment : segments) {
      // Check if the "displaySegmentsName" matches the specified segmentName
      if (segment.containsKey("displaySegmentsName") && segment["displaySegmentsName"].as<String>() == segmentName) {
        // Check if the displayUnits array exists within the segment
        if (segment.containsKey("displaySegmentLength")) {
          return segment["displaySegmentLength"].as<int>();
        }
      }
    }
  }

  // Return 0 if the requested data is not found
  return 0;
}


 /***************************************************************************************
 * Function to return the pin configuration based on the displaySegmentName
 * @param char* segmentName - name of the scoreboard segment e.g Top/Bottom
 * @return array of length 3 [0] = clockPin, [1]=dataPin, [2]=latchPin
 ****************************************************************************************/
int* SevenSegmentDisplayController::getPinsBySegmentName(const char* segmentName) {
  static int pins[3]; // Array to hold the pins: [clockPin, dataPin, latchPin]

  // Get the array of displaySegments
  JsonArray displaySegments = arduinoPinConfig_doc["displaySegments"].as<JsonArray>();

  // Search for the segment with the given name
  for (JsonObject segment : displaySegments) {
    if (segment["displaySegmentsName"].as<String>() == segmentName) {
      // Extract the pins and store them in the array
      pins[0] = segment["arduinoControlPins"]["clockPin"].as<int>();
      pins[1] = segment["arduinoControlPins"]["dataPin"].as<int>();
      pins[2] = segment["arduinoControlPins"]["latchPin"].as<int>();
      
      return pins;
    }
  }

  // Return NULL if the specified segment name was not found
  return NULL;
}


 /***************************************************************************************
 * Function to write what is to be displayed in a Segment on the Scoreboard
 * @param String - dataToDisplay - the string to display on the scoreboard
 * @param char* segmentName - name of the scoreboard segment e.g Top/Bottom
 ****************************************************************************************/
void SevenSegmentDisplayController::writeDataToDisplay(String dataToDisplay, const char* segmentName) {
  
  // Get the array of displaySegments
  JsonArray displaySegments = arduinoPinConfig_doc["displaySegments"].as<JsonArray>();

  // Initialize pin values to an invalid value
  int myLatchPin = -1;  
  int myDataPin = -1;   
  int myClockPin = -1;  
  
  // Initialize the segment length to 0
  int mySegmentLength = 0;  

  // Search for the segment with the given name
  for (JsonObject segment : displaySegments) {
    if (segment["displaySegmentsName"].as<String>() == segmentName) {
      // Set the Arduino pins
      myLatchPin = segment["arduinoControlPins"]["latchPin"].as<int>();
      myDataPin = segment["arduinoControlPins"]["dataPin"].as<int>();
      myClockPin = segment["arduinoControlPins"]["clockPin"].as<int>();
      //set the segment length
      mySegmentLength = segment["displaySegmentLength"].as<int>();
      break;  // Exit the loop since we found the segment
    }
  }

  // Check if the segment was found
  if (myLatchPin == -1 || myDataPin == -1 || myClockPin == -1|| mySegmentLength == 0) {
    // Handle the error (segment not found)
    Serial.println("Segment not found could not set PIN's or segment length");
    return;
  }

  // Format data to the correct Segment length
  dataToDisplay = displaySegmentFormater(dataToDisplay, segmentName);

  // Ground latchPin and hold low for as long as you are transmitting
  digitalWrite(myLatchPin, LOW);

  // Write data to the shift register
  for (int i = mySegmentLength - 1; i >= 0; i--) {
    String tempChar = String(dataToDisplay.charAt(i));
    shiftOut(myDataPin, myClockPin, LSBFIRST, binaryToDisplay(tempChar));
  }

  // Return the latch pin high to signal the chip that it
  // no longer needs to listen for information
  digitalWrite(myLatchPin, HIGH);
}