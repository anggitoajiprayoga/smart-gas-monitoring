void setup_sensor() {
  pinMode(PULSE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), onPulse, FALLING);
}

void loop_sensor() {
  handleMeterSave();
  handleFlowRateCalc();
  handleHourlyUsage();
  handleTodayUsage();
}

void handleMeterSave() {
  static unsigned long prevSaveMeter = 0;

  if (xmeter != meter && millis() - prevSaveMeter > 1000) {
    writeFile(SPIFFS, "/meter.txt", meter.c_str());
    xmeter = meter;
    prevSaveMeter = millis();
  }
}

void handleFlowRateCalc() {
  static unsigned long lastCalcTime = 0;
  static unsigned long lastPrintTime = 0;

  unsigned long now = millis();

  if (now - lastCalcTime >= FLOW_INTERVAL_MS) {
    unsigned long elapsedMs = now - lastCalcTime;
    lastCalcTime = now;

    portENTER_CRITICAL(&pulseMux);
    unsigned long count = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&pulseMux);

    if (elapsedMs > 0) {
      flowRateM3h = (count * GAS_PER_PULSE_M3) * (3600000.0 / elapsedMs);
    }

    if (now - lastPrintTime >= 5000) {
      Serial.print("Flow: ");
      Serial.print(flowRateM3h, 2);
      Serial.println(" m³/h");
      lastPrintTime = now;
    }
  }
}

void handleHourlyUsage() {
  static unsigned long prevRTCRead = 0;
  static RtcDateTime nowDT;

  unsigned long now = millis();
  if (now - prevRTCRead >= 1000) {
    nowDT = Rtc.GetDateTime();  // baca RTC maksimal 1x/detik
    prevRTCRead = now;
  }

  meter_final = meter_dm * 0.01;
  float remainingRaw = remaining_input.toFloat();

  if (remainingRaw > 0) {
    remaining_gas = (remainingRaw * 0.01) - (meter_final - remaining_meter.toFloat());
    if (remaining_gas < 0) remaining_gas = 0;
  } else {
    remaining_gas = 0;
  }

  if (datetime_start_hourly_usage == "" || meter_start_hourly_usage == "") {
    en_hourly_usage = "true";
    writeFile(SPIFFS, "/en_hourly_usage.txt", en_hourly_usage.c_str());
  }

  if (en_hourly_usage == "true") {
    datetime_start_hourly_usage = dateTime;
    meter_start_hourly_usage = meter_final;
    en_hourly_usage = "false";
    usage_hour_is_fixed = false;
    usage_hour_fixed = 0;

    writeFile(SPIFFS, "/datetime_start_hourly.txt", datetime_start_hourly_usage.c_str());
    writeFile(SPIFFS, "/meter_start_hourly_usage.txt", meter_start_hourly_usage.c_str());
    writeFile(SPIFFS, "/en_hourly_usage.txt", en_hourly_usage.c_str());
  }

  RtcDateTime startDT = parseDateTimeString(datetime_start_hourly_usage);
  uint32_t elapsedSeconds = nowDT - startDT;

  float usage_now = meter_final - meter_start_hourly_usage.toFloat();
  if (usage_now < 0) usage_now = 0;

  if (elapsedSeconds >= 3600 && !usage_hour_is_fixed) {
    usage_hour_fixed = usage_now;
    usage_hour_is_fixed = true;

    en_hourly_usage = "true";
    writeFile(SPIFFS, "/en_hourly_usage.txt", en_hourly_usage.c_str());
  }

  usage_hour = usage_hour_is_fixed ? usage_hour_fixed : usage_now;
}

void handleTodayUsage() {
  usage_today = meter_final - meter_start_today_usage.toFloat();
  if (usage_today < 0) usage_today = 0;

  if (datetime_start_today_usage != String(Day)) {
    en_today_usage = "true";
    writeFile(SPIFFS, "/en_today_usage.txt", en_today_usage.c_str());

    en_hourly_usage = "true";
    writeFile(SPIFFS, "/en_hourly_usage.txt", en_hourly_usage.c_str());
  }
}


RtcDateTime parseDateTimeString(String dtStr) {
  int year   = dtStr.substring(0, 4).toInt();
  int month  = dtStr.substring(5, 7).toInt();
  int day    = dtStr.substring(8, 10).toInt();
  int hour   = dtStr.substring(11, 13).toInt();
  int minute = dtStr.substring(14, 16).toInt();
  int second = dtStr.substring(17, 19).toInt();

  return RtcDateTime(year, month, day, hour, minute, second);
}
