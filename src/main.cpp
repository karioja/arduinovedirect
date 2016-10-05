
#include "Arduino.h"
#include "Vedirect.h"

#define VEDIRECT_RX 9
#define VEDIRECT_TX 8


#include <SoftwareSerial.h>
SoftwareSerial softwareSerial = SoftwareSerial(VEDIRECT_TX, VEDIRECT_RX);
//SoftwareSerial *debugSerial = &softwareSerial;
HardwareSerial *debugSerial = &Serial1;

HardwareSerial *vedirectSerial = &Serial;

Vedirect vedirect = Vedirect();

long lastStatsUpdate = 0;

void setup() {
  //while (!Serial);
  
  //Serial.begin(1200);
  vedirectSerial->begin(57600);
  debugSerial->begin(115200);

  debugSerial->println(F("Initializing Vedirect"));

  vedirect.initialize(*vedirectSerial, *debugSerial);
}


void loop() {
  vedirect.process();

  if (millis() > (lastStatsUpdate + 20000)) {
    VedirectData data = vedirect.getData();
    debugSerial->println(data.main_battery_voltage);
    debugSerial->println(data.battery_current);
    lastStatsUpdate = millis();
  }
  
  
}
