#ifndef Vedirect_h
#define Vedirect_h

#if defined(ARDUINO)
#include "Arduino.h"
#elif defined(SPARK)
#include "Particle.h"
#endif

#ifndef NUMBER_OF_SAMPLES
#define NUMBER_OF_SAMPLES 60
#endif


#define MAX_INPUT 128

#define STATE_WAIT_HEADER 0
#define STATE_IN_KEY 1
#define STATE_IN_VALUE 2
#define STATE_IN_CHECKSUM 3
#define STATE_HEX 4

#define prog_char_strcmp(a, b) strcmp_P((a), (b))
#define prog_char char PROGMEM
#define KEY(a, b) prog_char_strcmp((a), (prog_char *)F((b))) == 0
typedef const __FlashStringHelper *VedirectFlashStringPtr;


typedef struct {
  int main_battery_voltage[NUMBER_OF_SAMPLES];
  int average_main_battery_voltage;
  int aux_battery_voltage[NUMBER_OF_SAMPLES];
  int average_aux_battery_voltage;
  int panel_voltage[NUMBER_OF_SAMPLES];
  int average_panel_voltage;
  int panel_power[NUMBER_OF_SAMPLES];
  int average_panel_power;
  int battery_mid_point_voltage;
  int battery_mid_point_deviation;
  int battery_current[NUMBER_OF_SAMPLES];
  int average_battery_current;
  int load_current[NUMBER_OF_SAMPLES];
  int average_load_current;
  int battery_temperature;
  int state_of_charge;
  int time_to_go;
  int alarm_reason;
  int alarm_condition_active;
  int relay_state;
  int state_of_operation;
  int consumed_amp_hours;
  int instantaneous_power[NUMBER_OF_SAMPLES];
  int average_power;
  int deepest_discharge;
  int last_discharge;
  int average_discharge;
  int charge_cycles;
  int full_discharges;
  int cumulative_amp_hours_drawn;
  int minimum_battery_voltage;
  int maximum_battery_voltage;
  int seconds_since_last_full_charge;
  int automatic_syncronizations;
  int low_voltage_alarms;
  int high_voltage_alarms;
  int discharged_energy;
  int charged_energy;
  int yield_total;
  int yield_today;
  int maximum_power_today;
  int yield_yesterday;
  int maximum_power_yesterday;
  char product_id[8];
  char load_output_state[4];
  char error_code[4];
  char firmware_version[5];
} VedirectData;

class Vedirect
{
 public:
  Vedirect(Stream& serial);
  void initialize(Stream* serial);
  void process();
  uint8_t isDataAvailable();
  VedirectData getData();
  char * getVedirectData();
 protected:
  void processIncomingSerialByte(const int inByte);
  void collectData(char * key, char * value);
 private:
  void updateAverage();
  int calculateAverage(int *array);
  Stream *mySerial;
  char input_line [MAX_INPUT];
  char keybuffer[10];
  uint8_t keybufferindex;
  char valuebuffer[34];
  uint8_t valuebufferindex;
  uint8_t state;
  uint32_t bytes_sum;
  uint8_t data_valid;
  uint8_t buffer_position;
  char vedirectdata[256];
  VedirectData vedirectData;
};

#endif
