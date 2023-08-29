#ifndef SevenSegmentDisplayController_h
#define SevenSegmentDisplayController_h

// Check if Arduino.h is already included
#ifndef ARDUINO_H
  #include <Arduino.h>
#endif

// Check if ArduinoJson.h is already included
#ifndef ARDUINOJSON_H
  #include <ArduinoJson.h>
#endif



class SevenSegmentDisplayController {
public:
  SevenSegmentDisplayController();
  byte binaryToDisplay(String toDisplay);
  void initialiseArduinoPins();
  void writeDataToDisplay(String dataToDisplay, const char* segmentName);
  int getDisplayUnitLength(const char* segmentName, const char* unitName);
  int getDisplaySegmentLength(const char* segmentName);
  int* getPinsBySegmentName(const char* segmentName);
  String displaySegmentFormater(String dataToDisplay, const char* segmentName);
  String displayUnitFormater(String dataToDisplay, const char* segmentName, const char* unitName);

private:
  StaticJsonDocument<768> display_doc;
  StaticJsonDocument<768> scoreboardDisplayConfig_doc;
  JsonObject arduinoPinConfig_doc;
};

#endif
