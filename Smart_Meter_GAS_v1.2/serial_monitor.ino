unsigned long prevMillis = 0;
const unsigned long interval = 2000;

void serial_monitor() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillis >= interval) {
    prevMillis = currentMillis;
    
  Serial.println(F("========== GAS METER SYSTEM STATUS =========="));
  Serial.println(F("--- Meter Info ---"));
  Serial.print(F("Meter Raw         : ")); Serial.println(meter);
  Serial.print(F("Meter DM          : ")); Serial.println(meter_dm);
  Serial.print(F("Meter (Final)     : ")); Serial.print(meter_final, 2); Serial.println(F(" m³"));
  Serial.print(F("xMeter            : ")); Serial.println(xmeter);

  Serial.println(F("--- Remaining Gas ---"));
  Serial.print(F("Remaining Input   : ")); Serial.println(remaining_input);
  Serial.print(F("Remaining Meter   : ")); Serial.println(remaining_meter);
  Serial.print(F("Remaining Volume  : ")); Serial.print(remaining_gas, 2); Serial.println(F(" m³"));

  Serial.println(F("--- Flow Rate ---"));
  Serial.print(F("Pulse Count       : ")); Serial.println(pulseCount);
  Serial.print(F("Flow Rate         : ")); Serial.print(flowRateM3h, 2); Serial.println(F(" m³/h"));

  Serial.println(F("--- Usage Info ---"));
  Serial.print(F("Usage (Hour)      : ")); Serial.print(usage_hour, 2); Serial.println(F(" m³"));
  Serial.print(F("Usage (Today)     : ")); Serial.print(usage_today, 2); Serial.println(F(" m³"));

  Serial.println(F("--- Hourly Usage Tracking ---"));
  Serial.print(F("Start Time (Hr)   : ")); Serial.println(datetime_start_hourly_usage);
  Serial.print(F("Start Meter (Hr)  : ")); Serial.println(meter_start_hourly_usage);
  
  Serial.println(F("--- Daily Usage Tracking ---"));
  Serial.print(F("Start Date (Day)  : ")); Serial.println(datetime_start_today_usage);
  Serial.print(F("Start Meter (Day) : ")); Serial.println(meter_start_today_usage);
  Serial.print(F("Current Day       : ")); Serial.println(Day);
  Serial.print(F("Enable Daily Use  : ")); Serial.println(en_today_usage);

  Serial.println(F("--- Battery Info ---"));
  Serial.print(F("Voltage           : ")); Serial.print(battery_voltage); Serial.println(F(" V"));
  Serial.print(F("Battery Percent   : ")); Serial.print(battery_percent); Serial.println(F(" %"));
  Serial.print(F("Board Temp        : ")); Serial.print(board_temp); Serial.println(F(" °C"));

  Serial.println(F("--- Network Info ---"));
  Serial.print(F("SSID              : ")); Serial.println(ssid);
  Serial.print(F("IP Address        : ")); Serial.println(ip);
  Serial.print(F("MAC Address       : ")); Serial.println(mac);
  Serial.print(F("Gateway           : ")); Serial.println(gateway);
  Serial.print(F("Subnet Mask       : ")); Serial.println(subnet);
  Serial.print(F("RSSI              : ")); Serial.print(rssi); Serial.println(F(" dBm"));

  Serial.println(F("--- System Info ---"));
  Serial.print(F("Board Version     : ")); Serial.println(board_version);
  Serial.print(F("Firmware Version  : ")); Serial.println(firmware_version);
  Serial.print(F("Serial Number     : ")); Serial.println(serial_number);
  Serial.print(F("Assembly Date     : ")); Serial.println(assembly_date);

  Serial.println(F("--- Timestamps ---"));
  Serial.print(F("Current Time      : ")); Serial.println(dateTime);
  Serial.print(F("Last Update       : ")); Serial.println(last_update);
  Serial.print(F("Millis (now)      : ")); Serial.println(millis());

  Serial.println(F("--- ADS1115 Voltage Readings ---"));
  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println(" V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println(" V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println(" V");
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println(" V");

  Serial.println(F("========== KONFIGURASI =========="));
  Serial.print(F("Username                : ")); Serial.println(username);
  Serial.print(F("Password                : ")); Serial.println(password);
  Serial.print(F("en_client               : ")); Serial.println(en_client);
  Serial.print(F("SSID Client             : ")); Serial.println(ssid_client);
  Serial.print(F("Password Client         : ")); Serial.println(password_client);
  Serial.print(F("en_ap                   : ")); Serial.println(en_ap);
  Serial.print(F("SSID AP                 : ")); Serial.println(ssid_ap);
  Serial.print(F("Password AP             : ")); Serial.println(password_ap);
  Serial.print(F("en_alarm                : ")); Serial.println(en_alarm);
  Serial.print(F("Alarm Threshold         : ")); Serial.println(alarm_threshold_min);
  Serial.print(F("en_valve                : ")); Serial.println(en_valve);
  Serial.print(F("Valve Threshold         : ")); Serial.println(valve_threshold_min);
  Serial.print(F("Code                    : ")); Serial.println(code);
  Serial.print(F("URL OTA                 : ")); Serial.println(url_ota);
  Serial.println("alarm on: " + String(alarm_on) + " | interlock: " + String(interlock_alarm) + " | buzzer on: " + buzzer_on + " | buzzer_off: " + buzzer_off);
  
  Serial.println(F("=============================================\n"));
  }
}
