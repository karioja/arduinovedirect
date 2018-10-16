/*                                                                                                                                                                                                           Vedirect.cpp                                                                                                                                                                                             */

#include "Arduino.h"
#include "Vedirect.h"
#include <stdarg.h>

Vedirect::Vedirect(Stream& serial)
{
  initialize(&serial);
}

void Vedirect::initialize(Stream* serial)
{
  state = STATE_WAIT_HEADER;
  this->mySerial = serial;
}

// state machine
void Vedirect::process() {
  if (mySerial->available() > 0) {
    processIncomingSerialByte(mySerial->read());
    return;
  }
}

void Vedirect::collectData(char * key, char * value) {
  if (KEY(key, "V")) {
    vedirectData.main_battery_voltage[buffer_position] = atoi(value);
  } else if (KEY(key, "VS")) {
    vedirectData.aux_battery_voltage[buffer_position] = atoi(value);
  } else if (KEY(key, "VM")) {
    vedirectData.battery_mid_point_voltage = atoi(value);
  } else if (KEY(key, "DM")) {
    vedirectData.battery_mid_point_deviation = atoi(value);
  } else if (KEY(key, "I")) {
    vedirectData.battery_current[buffer_position] = atoi(value);
  } else if (KEY(key, "VPV")) {
    vedirectData.panel_voltage[buffer_position] = atoi(value);
  } else if (KEY(key, "PPV")) {
    vedirectData.panel_power[buffer_position] = atoi(value);
  } else if (KEY(key, "IL")) {
    vedirectData.load_current[buffer_position] = atoi(value);
  } else if (KEY(key, "SOC")) {
    vedirectData.state_of_charge = atoi(value);
  } else if (KEY(key, "TTG")) {
    // TODO handle "---"
    vedirectData.time_to_go = atoi(value);
  } else if (KEY(key, "T")) {
    // TODO handle "---"
    vedirectData.battery_temperature = atoi(value);
  } else if (KEY(key, "CS")) {
    vedirectData.state_of_operation = atoi(value);
  } else if (KEY(key, "CE")) {
    // TODO handle "---"
    vedirectData.consumed_amp_hours = atoi(value);
  } else if (KEY(key, "P")) {
    vedirectData.instantaneous_power[buffer_position] = atoi(value);
  } else if (KEY(key, "FW")) {
    strcpy(vedirectData.firmware_version, value);
  } else if (KEY(key, "PID")) {
    strcpy(vedirectData.product_id, value);
  } else if (KEY(key, "LOAD")) {
    strcpy(vedirectData.load_output_state, value);
  } else if (KEY(key, "H1")) {
    vedirectData.deepest_discharge = atoi(value);
  } else if (KEY(key, "H2")) {
    vedirectData.last_discharge = atoi(value);
  } else if (KEY(key, "H3")) {
    vedirectData.average_discharge = atoi(value);
  } else if (KEY(key, "H4")) {
    vedirectData.charge_cycles = atoi(value);
  } else if (KEY(key, "H5")) {
    vedirectData.full_discharges = atoi(value);
  } else if (KEY(key, "H6")) {
    vedirectData.cumulative_amp_hours_drawn = atoi(value);
  } else if (KEY(key, "H7")) {
    vedirectData.minimum_battery_voltage = atoi(value);
  } else if (KEY(key, "H8")) {
    vedirectData.maximum_battery_voltage = atoi(value);
  } else if (KEY(key, "H9")) {
    vedirectData.seconds_since_last_full_charge = atoi(value);
  } else if (KEY(key, "H10")) {
    vedirectData.automatic_syncronizations = atoi(value);
  } else if (KEY(key, "H11")) {
    vedirectData.low_voltage_alarms = atoi(value);
  } else if (KEY(key, "H12")) {
    vedirectData.high_voltage_alarms = atoi(value);
  } else if (KEY(key, "H17")) {
    vedirectData.discharged_energy = atoi(value);
  } else if (KEY(key, "H18")) {
    vedirectData.charged_energy = atoi(value);
  } else if (KEY(key, "H19")) {
    vedirectData.yield_total = atoi(value);
  } else if (KEY(key, "H20")) {
    vedirectData.yield_today = atoi(value);
  } else if (KEY(key, "H21")) {
    vedirectData.maximum_power_today = atoi(value);
  } else if (KEY(key, "H22")) {
    vedirectData.yield_yesterday = atoi(value);
  } else if (KEY(key, "H23")) {
    vedirectData.maximum_power_yesterday = atoi(value);
  } else if (KEY(key, "ERR")) {
    strcpy(vedirectData.error_code, value);
  } else if (KEY(key, "Alarm")) {
    vedirectData.alarm_condition_active = KEY(value, "OFF");
  } else if (KEY(key, "Relay")) {
    vedirectData.relay_state = KEY(value, "OFF");
  } else if (KEY(key, "AR")) {
    vedirectData.alarm_reason = atoi(value);
  }
}

void Vedirect::updateAverage() {
  vedirectData.average_power = calculateAverage(vedirectData.instantaneous_power);
  vedirectData.average_load_current = calculateAverage(vedirectData.load_current);
  vedirectData.average_battery_current = calculateAverage(vedirectData.battery_current);
  vedirectData.average_panel_power = calculateAverage(vedirectData.panel_power);
  vedirectData.average_panel_voltage = calculateAverage(vedirectData.panel_voltage);
  vedirectData.average_main_battery_voltage = calculateAverage(vedirectData.main_battery_voltage);
  vedirectData.average_aux_battery_voltage = calculateAverage(vedirectData.aux_battery_voltage);
}

void Vedirect::processIncomingSerialByte(const int inByte) {
  if (inByte == ':' && state != STATE_IN_CHECKSUM) {
    state = STATE_HEX;
  }

  switch(state) {
  case STATE_WAIT_HEADER:
    data_valid = 0;
    if (inByte == 0x0d) { // \r
      bytes_sum += inByte;
      state = STATE_WAIT_HEADER;
    } else if (inByte == 0x0a) { // \n
      bytes_sum += inByte;
      state = STATE_IN_KEY;
      keybufferindex = 0;
      return;
    }
    break;
  case STATE_IN_KEY:
    bytes_sum += inByte;
    if (inByte == 9) { // \t
      keybuffer[keybufferindex++] = 0;
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
    bytes_sum += inByte;
    if (inByte == 0x0d) { // \r
      state = STATE_WAIT_HEADER;
      valuebuffer[valuebufferindex++] = 0;
      // collect data into VedirectData struct
      collectData(keybuffer, valuebuffer);

    } else {
      valuebuffer[valuebufferindex++] = inByte;
    }

    break;
  case STATE_IN_CHECKSUM:
    bytes_sum += inByte;
    if (bytes_sum % 256 == 0) {
      data_valid = 1;
      buffer_position++;
      if (buffer_position == NUMBER_OF_SAMPLES) {
        buffer_position = 0;
      }
      updateAverage();
    }
    bytes_sum = 0;
    state = STATE_WAIT_HEADER;
    break;
  case STATE_HEX:
    data_valid = 0;
    if (inByte == 0x0a) {
      // end of hex
      bytes_sum = 0;
      state = STATE_WAIT_HEADER;
    }
    break;
  default:
    state = STATE_WAIT_HEADER;
  }

  return;
}

int Vedirect::calculateAverage(int *array) {
  int sum = 0;
  int avg = 0;
  for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
    sum = sum+*(array+i);
  }
  avg = sum / NUMBER_OF_SAMPLES;
  return avg;
}

uint8_t Vedirect::isDataAvailable() {
  return data_valid;
}

VedirectData Vedirect::getData() {
  return vedirectData;
}

char * Vedirect::getVedirectData() {
  if (KEY(vedirectData.product_id, "0x203")) {
    sprintf(vedirectdata, "\r\nV\t%d\r\nVS\t%d\r\nPID\t%s\r\nSOC\t%d\r\nI\t%d\r\nP\t%d\r\nH7\t%d\r\nH8\t%d\r\nH9\t%d\r\nCE\t%d\r\nChecksum\t",
	    vedirectData.average_main_battery_voltage,
	    vedirectData.average_aux_battery_voltage,
	    vedirectData.product_id,
	    vedirectData.state_of_charge,
	    vedirectData.average_battery_current,
	    vedirectData.average_power,
	    vedirectData.minimum_battery_voltage,
	    vedirectData.maximum_battery_voltage,
	    vedirectData.seconds_since_last_full_charge,
	    vedirectData.consumed_amp_hours);
  } else if (KEY(vedirectData.product_id, "0xA042")) {
    sprintf(vedirectdata, "\r\nPID\t%s\r\nV\t%d\r\nI\t%d\r\nVPV\t%d\r\nPPV\t%d\r\nCS\t%d\r\nIL\t%d\r\nH19\t%d\r\nH20\t%d\r\nH21\t%d\r\nH22\t%d\r\nH23\t%d\r\nChecksum\t",
	    vedirectData.product_id,
	    vedirectData.average_main_battery_voltage,
	    vedirectData.average_battery_current,
	    vedirectData.average_panel_voltage,
	    vedirectData.average_panel_power,
	    vedirectData.state_of_operation,
	    vedirectData.average_load_current,
	    vedirectData.yield_total,
	    vedirectData.yield_today,
	    vedirectData.maximum_power_today,
	    vedirectData.yield_yesterday,
	    vedirectData.maximum_power_yesterday);
  }

  uint32_t sum = 0;
  uint8_t i = 0;
  for (; i < strlen(vedirectdata); i++) {
    sum += vedirectdata[i];
  }
  uint8_t checksum = 256 - sum % 256;
  vedirectdata[i] = checksum;
  vedirectdata[i+1] = '\0';

  return vedirectdata;
}
