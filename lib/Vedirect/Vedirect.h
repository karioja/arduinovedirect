#ifndef Vedirect_h
#define Vedirect_h

#define DebugStream Serial
#define DEBUG_PRINT(...) DebugStream.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) DebugStream.println(__VA_ARGS__)



#include "Arduino.h"

#define MAX_INPUT 128

#define STATE_WAIT_HEADER 0
#define STATE_IN_KEY 1
#define STATE_IN_VALUE 2
#define STATE_IN_CHECKSUM 3

#define prog_char_strcmp(a, b) strcmp_P((a), (b))
#define prog_char  char PROGMEM
typedef const __FlashStringHelper *VedirectFlashStringPtr;

typedef struct {
  uint32_t main_battery_voltage;
  uint32_t aux_battery_voltage;
  uint32_t panel_voltage;
  uint32_t panel_power;
  uint32_t battery_mid_point_voltage;
  uint32_t battery_mid_point_deviation;
  int32_t battery_current;
  int32_t load_current;
  int8_t battery_temperature;
  uint32_t state_of_charge;
  uint8_t alarm_condition_active;
  uint8_t relay_state;
} VedirectData;

class Vedirect
{
 public:
  Vedirect();
  void initialize(Stream &serial, Stream &debugStream);
  void process();
  uint8_t isDataAvailable();
  VedirectData getData();
 protected:
  void processIncomingSerialByte(const int inByte);
  void collectData(char * key, char * value);
 private:
  void updateAverage();
  Stream *mySerial;
  Stream *debugSerial;
  char input_line [MAX_INPUT];
  char keybuffer[10];
  uint8_t keybufferindex;
  char valuebuffer[34];
  uint8_t valuebufferindex;
  uint8_t state;
  uint32_t bytes_sum;
  uint8_t data_valid;
  VedirectData vedirectData;
  VedirectData averageData;
};

#endif
