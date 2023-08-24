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
  String displayUnitFormater(int displayUnitLength, String dataToDisplay);
  void SevenSegmentDisplayController::initialiseArduinoPins();

private:
  StaticJsonDocument<768> display_doc;
  StaticJsonDocument<256> scoreboardDisplayConfig_doc;
  JsonObject arduinoPinConfig_doc;
};

#endif
