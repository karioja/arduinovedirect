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
    vedirectData.main_battery_voltage = atoi(value);
  } else if (KEY(key, "VS")) {
    vedirectData.aux_battery_voltage = atoi(value);
  } else if (KEY(key, "VM")) {
    vedirectData.battery_mid_point_voltage = atoi(value);
  } else if (KEY(key, "DM")) {
    vedirectData.battery_mid_point_deviation = atoi(value);
  } else if (KEY(key, "I")) {
    vedirectData.battery_current = atoi(value);
  } else if (KEY(key, "VPV")) {
    vedirectData.panel_voltage = atoi(value);
  } else if (KEY(key, "PPV")) {
    vedirectData.panel_power = atoi(value);
  } else if (KEY(key, "IL")) {
    vedirectData.load_current = atoi(value);
  } else if (KEY(key, "SOC")) {
    if (KEY(value, "---")) {
      vedirectData.state_of_charge = -1;
    } else {
      vedirectData.state_of_charge = atoi(value);
    }
  } else if (KEY(key, "TTG")) {
    if (KEY(value, "---")) {
      vedirectData.time_to_go = -1;
    } else {
      vedirectData.time_to_go = atoi(value);
    }
  } else if (KEY(key, "T")) {
    if (KEY(value, "---")) {
      vedirectData.battery_temperature = -1;
    } else {
      vedirectData.battery_temperature = atoi(value);
    }
  } else if (KEY(key, "CS")) {
    vedirectData.state_of_operation = atoi(value);
  } else if (KEY(key, "CE")) {
    if (KEY(value, "---")) {
      vedirectData.consumed_amp_hours = -1;
    } else {
      vedirectData.consumed_amp_hours = atol(value);
    }
  } else if (KEY(key, "P")) {
    vedirectData.instantaneous_power = atoi(value);
  } else if (KEY(key, "FW")) {
    strcpy(vedirectData.firmware_version, value);
  } else if (KEY(key, "PID")) {
    strcpy(vedirectData.product_id, value);
  } else if (KEY(key, "LOAD")) {
    strcpy(vedirectData.load_output_state, value);
  } else if (KEY(key, "H1")) {
    vedirectData.deepest_discharge = atol(value);
  } else if (KEY(key, "H2")) {
    vedirectData.last_discharge = atol(value);
  } else if (KEY(key, "H3")) {
    vedirectData.average_discharge = atol(value);
  } else if (KEY(key, "H4")) {
    vedirectData.charge_cycles = atoi(value);
  } else if (KEY(key, "H5")) {
    vedirectData.full_discharges = atoi(value);
  } else if (KEY(key, "H6")) {
    vedirectData.cumulative_amp_hours_drawn = atol(value);
  } else if (KEY(key, "H7")) {
    vedirectData.minimum_battery_voltage = atoi(value);
  } else if (KEY(key, "H8")) {
    vedirectData.maximum_battery_voltage = atoi(value);
  } else if (KEY(key, "H9")) {
    vedirectData.seconds_since_last_full_charge = atol(value);
  } else if (KEY(key, "H10")) {
    vedirectData.automatic_synchronizations = atoi(value);
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
    vedirectData.alarm_condition_active = !KEY(value, "OFF");
  } else if (KEY(key, "Relay")) {
    vedirectData.relay_state = !KEY(value, "OFF");
  } else if (KEY(key, "AR")) {
    vedirectData.alarm_reason = atoi(value);
  } else if (KEY(key, "MPPT")) {
    vedirectData.tracker_operation_mode = atoi(value);
  } else if (KEY(key, "HSDS")) {
    vedirectData.day_sequence_number = atoi(value);
  }
}

void Vedirect::processIncomingSerialByte(const int inByte) {
  //Serial.print((char)inByte);
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

uint8_t Vedirect::isDataAvailable() {
  return data_valid;
}

VedirectData Vedirect::getData() {
  return vedirectData;
}

char * Vedirect::getVedirectData() {
  if (KEY(vedirectData.product_id, "0x203")) {
    sprintf(vedirectdata, "\r\nV\t%d\r\nVS\t%d\r\nPID\t%s\r\nSOC\t%d\r\nI\t%d\r\nP\t%d\r\nH7\t%d\r\nH8\t%d\r\nH9\t%d\r\nCE\t%d\r\nChecksum\t",
	    vedirectData.main_battery_voltage,
	    vedirectData.aux_battery_voltage,
	    vedirectData.product_id,
	    vedirectData.state_of_charge,
	    vedirectData.battery_current,
	    vedirectData.panel_power,
	    vedirectData.minimum_battery_voltage,
	    vedirectData.maximum_battery_voltage,
	    vedirectData.seconds_since_last_full_charge,
	    vedirectData.consumed_amp_hours);
  } else if (KEY(vedirectData.product_id, "0xA042")) {
    sprintf(vedirectdata, "\r\nPID\t%s\r\nV\t%d\r\nI\t%d\r\nVPV\t%d\r\nPPV\t%d\r\nCS\t%d\r\nIL\t%d\r\nH19\t%d\r\nH20\t%d\r\nH21\t%d\r\nH22\t%d\r\nH23\t%d\r\nChecksum\t",
	    vedirectData.product_id,
	    vedirectData.main_battery_voltage,
	    vedirectData.battery_current,
	    vedirectData.panel_voltage,
	    vedirectData.panel_power,
	    vedirectData.state_of_operation,
	    vedirectData.load_current,
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
