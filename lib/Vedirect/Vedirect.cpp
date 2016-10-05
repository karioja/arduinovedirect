/*                                                                                                                                                                                                           Vedirect.cpp                                                                                                                                                                                             */

#include "Arduino.h"
#include "Vedirect.h"

Vedirect::Vedirect()
{
  state = STATE_WAIT_HEADER;
}

void Vedirect::initialize(Stream &serial, Stream &debugStream)
{
  mySerial = &serial;
  debugSerial = &debugStream;
}

// state machine
void Vedirect::process() {
  while(mySerial->available()) {
    processIncomingSerialByte(mySerial->read());
  }
}

void Vedirect::collectData(char * key, char * value) {
  if (prog_char_strcmp(key, (prog_char *)F("V")) == 0) {
    vedirectData.main_battery_voltage = atol(value);
  } else if (prog_char_strcmp(key, (prog_char *)F("I")) == 0) {
    vedirectData.battery_current = atol(value);
  } else if (prog_char_strcmp(key, (prog_char *)F("VPV")) == 0) {
    vedirectData.panel_voltage = atol(value);
  } else if (prog_char_strcmp(key, (prog_char *)F("PPV")) == 0) {
    vedirectData.panel_power = atol(value);
  } else if (prog_char_strcmp(key, (prog_char *)F("IL")) == 0) {
    vedirectData.load_current = atol(value);
  }
}

void Vedirect::updateAverage() {
  //debugSerial->println(0.05 * vedirectData.main_battery_voltage);
  //debugSerial->println(0.95 * averageData.main_battery_voltage);
  averageData.main_battery_voltage = 0.05 * vedirectData.main_battery_voltage + 0.95 * averageData.main_battery_voltage;
  averageData.battery_current = 0.05 * vedirectData.battery_current + 0.95 * averageData.battery_current;
}

void Vedirect::processIncomingSerialByte(const int inByte) {
  switch(state) {
  case STATE_WAIT_HEADER:
    //debugSerial->print(F("STATE_WAIT_HEADER: "));
    //debugSerial->println(inByte);
    bytes_sum += inByte;
    if (inByte == 0x0d) { // \r
      state = STATE_WAIT_HEADER;
    } else if (inByte == 0x0a) { // \n
      state = STATE_IN_KEY;
      keybufferindex = 0;
      return;
    }
    break;
  case STATE_IN_KEY:
    //debugSerial->println(F("STATE_IN_KEY"));
    bytes_sum += inByte;
    if (inByte == 9) { // \t
      keybuffer[keybufferindex++] = 0;
      //debugSerial->println(keybuffer);
      if (prog_char_strcmp(keybuffer, (prog_char *)F("Checksum")) == 0) {
	state = STATE_IN_CHECKSUM;
      } else {
	state = STATE_IN_VALUE;
	valuebufferindex = 0;
      }
    } else {
      keybuffer[keybufferindex++] = inByte;
    }
    break;
  case STATE_IN_VALUE:
    //debugSerial->println(F("STATE_IN_VALUE"));
    bytes_sum += inByte;
    if (inByte == 0x0d) { // \r
      state = STATE_WAIT_HEADER;
      valuebuffer[valuebufferindex++] = 0;
      // collect data into VedirectData struct
      //debugSerial->println(valuebuffer);

      collectData(keybuffer, valuebuffer);
      
    } else {
      valuebuffer[valuebufferindex++] = inByte;
    }
    
    break;
  case STATE_IN_CHECKSUM:
    //debugSerial->println(F("STATE_IN_CHECKSUM"));
    bytes_sum += inByte;
    if (bytes_sum % 256 == 0) {
      data_valid = 1;
      //debugSerial->print(F("Correct checksum: "));
      //debugSerial->println(bytes_sum % 256);

      updateAverage();

      //debugSerial->print(F("Average main battery voltage: "));
      //debugSerial->println(averageData.main_battery_voltage);
      
    } else {
      //debugSerial->print(F("Invalid checksum: "));
      //debugSerial->println(bytes_sum % 256);
    }
    bytes_sum = 0;
    state = STATE_WAIT_HEADER;
    break;
  default:
    state = STATE_WAIT_HEADER;
  }

  return;
}

uint8_t Vedirect::isDataAvailable() {
  return data_valid;
}

VedirectData Vedirect::getData() {
  return averageData;
}
