#include "SevenSegmentDisplayController.h"


SevenSegmentDisplayController::SevenSegmentDisplayController() {
  // Constructor code goes here, initialize your JSON document here
  // Deserialize the JSON document in the constructor
  const char* display_json = "{\"0\": \"0xFC\",\"1\": \"0x60\",\"2\": \"0xDA\",\"3\": \"0xF2\",\"4\": \"0x66\",\"5\": \"0xB6\",\"6\": \"0xBE\",\"7\": \"0xE0\",\"8\": \"0xFE\",\"9\": \"0xE6\",\"a\": \"0xFA\",\"b\": \"0x3E\",\"c\": \"0x1A\",\"d\": \"0x7A\",\"e\": \"0x9E\",\"f\": \"0x8E\",\"g\": \"0xBC\",\"h\": \"0x2E\",\"i\": \"0xA0\",\"j\": \"0xB0\",\"k\": \"0xAE\",\"l\": \"0x1C\",\"m\": \"0xAA\",\"n\": \"0x2A\",\"o\": \"0x3A\",\"p\": \"0xCE\",\"q\":\"0xE6\",\"r\": \"0xA\",\"s\": \"0xB4\",\"t\": \"0x1E\",\"u\": \"0x38\",\"v\": \"0x54\",\"w\": \"0x56\",\"x\": \"0x28\",\"y\": \"0x76\",\"z\": \"0xD8\",\"*\": \"0x1\",\"#\": \"0x0\"}";
  deserializeJson(display_doc, display_json);

  const char* scoreboardDisplayConfig_json = "{\"numberArduinoPinSetups\": 2, \"arduinoControlPins\":[{\"clockPin\" : 2, \"dataPin\" : 3, \"latchPin\" : 4}, {\"clockPin\" : 10, \"dataPin\" : 11, \"latchPin\" : 12}]}";
  DeserializationError error = deserializeJson(scoreboardDisplayConfig_doc, scoreboardDisplayConfig_json);
  if (error) {
    Serial.print(F("deserialize arduinoControlPinConfig_json failed: "));
    Serial.println(error.f_str());
    return;
  }
  arduinoPinConfig_doc = scoreboardDisplayConfig_doc.as<JsonObject>();

}

/*
 **************************************************************************************
 * SevenSegmentDisplayController
 * 
 **************************************************************************************
*/
byte SevenSegmentDisplayController::binaryToDisplay(String toDisplay) {
  String byteString = display_doc[toDisplay];
  byte resultByte = (byte)strtol(byteString.c_str(), NULL, 16);
  return resultByte;
}

/*
 **************************************************************************************
 * Format the dataToDisplay to the correct display unit length.
 * @param displayUnitLength - the number of 7SegmentDisplays in the unit e.g. Bastman 1 has 3 7Segment Displays
 * @param dataToDisplay - the data string to display
 * @return String - of a length == to the displayUnitLength. 
 *         If dataToDisplay was to long return digits from the left. 
 *         If dataToDisplay was to short fill from the left with #
 **************************************************************************************
*/
String SevenSegmentDisplayController::displayUnitFormater(int displayUnitLength, String dataToDisplay){

  // Write data to the display
  dataToDisplay.trim();
  int dataToDisplayLength = dataToDisplay.length();

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
 * Initialise the Arduino Output Pins
 *  Takes setup data defined in the scoreboardDisplayConfig_json
 ****************************************************************************************/
void SevenSegmentDisplayController::initialiseArduinoPins(){

    for (int i = 0; i < arduinoPinConfig_doc["numberArduinoPinSetups"]; i++) {
    pinMode(arduinoPinConfig_doc["arduinoControlPins"][i]["clockPin"], OUTPUT);
      Serial.print("clockPin OUTPUT: ");
      Serial.println(arduinoPinConfig_doc["arduinoControlPins"][i]["clockPin"].as<String>());
    pinMode(arduinoPinConfig_doc["arduinoControlPins"][i]["dataPin"], OUTPUT);
      Serial.print("dataPin OUTPUT: ");
      Serial.println(arduinoPinConfig_doc["arduinoControlPins"][i]["dataPin"].as<String>());
    pinMode(arduinoPinConfig_doc["arduinoControlPins"][i]["latchPin"], OUTPUT);
      Serial.print("latchPin OUTPUT: ");
      Serial.println(arduinoPinConfig_doc["arduinoControlPins"][i]["latchPin"].as<String>());
      }

}