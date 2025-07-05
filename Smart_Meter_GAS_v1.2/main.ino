void setup_main() {
  pinMode(pin_proses, OUTPUT);
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_wifi_ap, OUTPUT);
  pinMode(pin_wifi_sta, OUTPUT);
  pinMode(pin_ind_selenoid, OUTPUT);
  pinMode(pin_selenoid, OUTPUT);
  digitalWrite(pin_buzzer, HIGH);
  delay(150);
  digitalWrite(pin_buzzer, LOW);
}

void parse_websocket(String input) {
  Serial.println();
  Serial.println("-----Payload Parse-----");
  Serial.println();

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, input);
  DeserializationError error = deserializeJson(doc, input);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  JsonObject obj = doc.as<JsonObject>();

  String startCodeVariable = readFile(SPIFFS, "/start_code.txt");
  String stopCodeVariable = readFile(SPIFFS, "/stop_code.txt");
  String setDateStr = readFile(SPIFFS, "/set_date.txt");
  String setTimeStr = readFile(SPIFFS, "/set_time.txt");

  auto tryUpdate = [&](const char* key, String & variable, const char* filePath) {
    if (obj.containsKey(key)) {
      String value = obj[key].as<String>();
      Serial.printf("tryUpdate key: %s, value: %s\n", key, value.c_str());

      if (value != "null" and value != "") {
        variable = value;
        writeFile(SPIFFS, filePath, value.c_str());
        Serial.printf("Updated [%s] -> %s\n", key, value.c_str());

        if (strcmp(key, "start_code") == 0) {
          Serial.println("Processing start_code...");
          if (value == code) {
            notifyClients("{\"selenoid_start_status\":\"success\"}");
            start_selenoid();
            Serial.println("✅ Selenoid Started");
          } else {
            notifyClients("{\"selenoid_start_status\":\"failed\"}");
            Serial.println("❌ Selenoid Can't Start - Incorrect Code");
          }
        }

        if (strcmp(key, "stop_code") == 0) {
          Serial.println("Processing off_code...");
          if (value == code) {
            notifyClients("{\"selenoid_stop_status\":\"success\"}");
            stop_selenoid();
            Serial.println("✅ Selenoid Stopped");
          } else {
            notifyClients("{\"selenoid_stop_status\":\"failed\"}");
            Serial.println("❌ Selenoid Can't Stop - Incorrect Code");
          }
        }

        if (strcmp(key, "set_date") == 0 or strcmp(key, "set_time") == 0) {
          String dateStr = readFile(SPIFFS, "/set_date.txt"); // Format: YYYY-MM-DD
          String timeStr = readFile(SPIFFS, "/set_time.txt"); // Format: HH:MM

          if (dateStr.length() >= 10 and timeStr.length() >= 5) {
            int year   = dateStr.substring(0, 4).toInt();
            int month  = dateStr.substring(5, 7).toInt();
            int day    = dateStr.substring(8, 10).toInt();
            int hour   = timeStr.substring(0, 2).toInt();
            int minute = timeStr.substring(3, 5).toInt();
            int second = 0;

            Rtc.SetDateTime(RtcDateTime(year, month, day, hour, minute, second));

            Serial.println("\n===============================");
            Serial.println("Set RTC Time Success");
            Serial.printf("RTC Set to: %04d-%02d-%02d %02d:%02d:%02d\n",
                          year, month, day, hour, minute, second);
            Serial.println("===============================\n");
          }
          else {
            Serial.println("Date/time format invalid.");
          }
        }

        if (strcmp(key, "url_ota") == 0) {
          Serial.println("Triggering OTA from URL...");
          ota();
        }

        if (strcmp(key, "remaining") == 0) {
          remaining_meter = meter_final;
          writeFile(SPIFFS, "/remaining_meter.txt", remaining_meter.c_str());
        }
        if (strcmp(key, "meter") == 0) {
          meter_final = meter.toFloat();
          meter_dm = meter.toInt();
          xmeter = meter;
        }
      }
    }
  };

  tryUpdate("set_date", setDateStr, "/set_date.txt");
  tryUpdate("set_time", setTimeStr, "/set_time.txt");
  tryUpdate("start_code", startCodeVariable, "/start_code.txt");
  tryUpdate("stop_code", stopCodeVariable, "/stop_code.txt");
  tryUpdate("meter", meter, "/meter.txt");
  tryUpdate("remaining", remaining_input, "/remaining_input.txt");
  tryUpdate("alarm_threshold", en_alarm, "/en_alarm.txt");
  tryUpdate("alarm_threshold_min", alarm_threshold_min, "/alarm_threshold_min.txt");
  tryUpdate("valve_threshold_enable", en_valve, "/en_valve.txt");
  tryUpdate("valve_threshold_min", valve_threshold_min, "/valve_threshold_min.txt");
  tryUpdate("wifi_client", en_client, "/en_client.txt");
  tryUpdate("ssid_client", ssid_client, "/ssid_client.txt");
  tryUpdate("password_client", password_client, "/password_client.txt");
  tryUpdate("wifi_ap", en_ap, "/en_ap.txt");
  tryUpdate("ssid_ap", ssid_ap, "/ssid_ap.txt");
  tryUpdate("password_ap", password_ap, "/password_ap.txt");
  tryUpdate("username", username, "/username.txt");
  tryUpdate("password", password, "/password.txt");
  tryUpdate("kode", code, "/code.txt");
  tryUpdate("url_ota", url_ota, "/url_ota.txt");
}

void loop_main() {
  if (pulseFlag) {
    portENTER_CRITICAL(&pulseMux);
    pulseFlag = false;
    unsigned long currentTime = millis();  // sekarang boleh pakai millis()
    portEXIT_CRITICAL(&pulseMux);

    // Logika asli kamu tetap dipakai:
    pulseCount++;
    meter_dm++;
    meter = String(meter_dm);             // ini dilakukan di loop, aman
    lastInterruptTime = currentTime;

    digitalWrite(pin_proses, HIGH);       // aman dilakukan di loop
    ledOnTime = currentTime;
    ledState = true;
  }

  int adcValue = ads.readADC_SingleEnded(3);
  bool button = (adcValue > 500);

  if (button and !lastButtonState and en_menu == 0) {
    en_menu = 4;
    ap_button_mode = true;
    ap_button_mode_timer = millis();
    lcd.clear();
    display_ap_toggle_hint();
    lastKeyPressTime = millis();
    buzzer_beep();

    if (alarm_on == true) {
      interlock_alarm = false;
      button_pause = true;
      alarm_snooze_start = millis();
      prev_buz_on = millis();
    }

    if (!backlightOn) {
      lcd.backlight();
      backlightOn = true;
    }
  }
  lastButtonState = button;

  if (en_menu == 4 and millis() - ap_button_mode_timer > AP_BUTTON_MODE_TIMEOUT) {
    if (!button) {  // hanya reset menu jika tombol dilepas
      en_menu = 0;
      ap_button_mode = false;
      lcd.clear();
    }
  }


  if (ledState and millis() - ledOnTime > 50) {
    digitalWrite(pin_proses, LOW);
    ledState = false;
  }

  if (backlightOn and millis() - lastKeyPressTime > backlightTimeout) {
    lcd.noBacklight();
    backlightOn = false;
  }

  if (en_menu == 0) {
    //    unsigned long now = millis();
    //    if (now - lastDisplaySwitch > displayInterval) {
    //      showMain1 = !showMain1;
    //      lastDisplaySwitch = now;
    //      lcd.clear();  // Bersihkan agar tidak tumpang tindih
    //    }

    if (showMain1) {
      dispay_main1();
    }
    else {
      dispay_main2();
    }
  }

  static unsigned long prev_read_threshold;
  if ((millis() - prev_read_threshold) > 2000) {
    alarm_threshold_min = String(readFile(SPIFFS, "/alarm_threshold_min.txt").toFloat() * 0.01);
    valve_threshold_min = String(readFile(SPIFFS, "/valve_threshold_min.txt").toFloat() * 0.01);
    prev_read_threshold = millis();
  }

  // ====== ALARM LOGIC ======
  if (en_alarm == "enable") {
    if (remaining_gas <= alarm_threshold_min.toFloat()) {
      alarm_on = true;
    }
    else {
      alarm_on = false;
      prev_buz_on = millis();
    }
  }
  else {
    alarm_on = false;
  }

  if ((millis() - alarm_snooze_start) > ALARM_SNOOZE_DURATION) {
    interlock_alarm = true;
    if (button_pause == true) {
      buzzer_on = true;
      buzzer_off = false;
      prev_buz_on = millis();
      button_pause = false;
    }
  }

  // ===== VALVE CONTROL =====
  if (en_valve == "enable") {
    if (remaining_gas <= valve_threshold_min.toFloat()) {
      stop_selenoid();
    }
  }
}

void waitForResetButton() {
  unsigned long start = millis();
  while (millis() - start < 6000) {
    reset_button();     // Cek tombol
    delay(50);          // Jangan terlalu cepat polling
  }
}

int lastCountdown = 0;
void reset_button() {
  int adcValue = ads.readADC_SingleEnded(3);
  bool button = (adcValue > 500);

  if (button and !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = millis();
    lastCountdown = HOLD_TIME / 1000;
    buzzer_stop();
  }

  if (button and buttonPressed) {
    unsigned long heldTime = millis() - buttonPressStart;

    int countdown = (HOLD_TIME - heldTime) / 1000 + 1;
    if (countdown != lastCountdown and countdown > 0) {
      Serial.print("Menahan tombol selama: ");
      Serial.print(5 - countdown + 1);
      Serial.print(" detik... (");
      Serial.print(countdown);
      Serial.println(")");
      display_button_reset(countdown);
      lastCountdown = countdown;
    }

    if (heldTime >= HOLD_TIME) {
      display_reset();
      delay(1500);
      data_default();
      Serial.println("TOMBOL DITEKAN 5 DETIK, RESET!");
      delay(1000);
      ESP.restart();
      buttonPressed = false;
    }
  }

  if (!button and buttonPressed) {
    buttonPressed = false;
    buzzer_stop();
  }
}

void loop_ads() {
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);

  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);

  battery_voltage = adc0 * 0.01505;
  battery_percent = (battery_voltage - 3.0) / 1.2 * 100.0;
  battery_percent = round(battery_percent);  // Bulatkan ke 100 jika nilainya 99.5+
  battery_percent = constrain(battery_percent, 0, 100);

  //  battery_voltage = adc0 * 0.01505;  // Pastikan ini benar konversinya
  //  battery_percent = (battery_voltage - 2.0) / (4.0 - 2.0) * 100.0;
  //  battery_percent = constrain(battery_percent, 0, 100);
}

bool gasHabisTriggered = false;  // Global, di luar fungsi

void start_selenoid() {
  if (en_valve == "enable") {
    if (remaining_gas <= valve_threshold_min.toFloat()) {
      digitalWrite(pin_selenoid, LOW);
      digitalWrite(pin_ind_selenoid, LOW);

      if (!gasHabisTriggered) {
        gas_habis();                 // Panggil hanya sekali
        gasHabisTriggered = true;   // Set flag
      }
    }
    else {
      digitalWrite(pin_selenoid, HIGH);
      digitalWrite(pin_ind_selenoid, HIGH);
      gasHabisTriggered = false;  // Reset flag kalau gas sudah kembali normal
    }
  }
  else {
    digitalWrite(pin_selenoid, HIGH);
    digitalWrite(pin_ind_selenoid, HIGH);
    gasHabisTriggered = false;  // Reset juga jika valve dinonaktifkan
  }
}


void stop_selenoid() {
  digitalWrite(pin_selenoid, LOW);
  digitalWrite(pin_ind_selenoid, LOW);
}

void ind_wifi_ap_on() {
  digitalWrite(pin_wifi_ap, HIGH);
}

void ind_wifi_ap_off() {
  digitalWrite(pin_wifi_ap, LOW);
}

void ind_wifi_sta_on() {
  digitalWrite(pin_wifi_sta, HIGH);
}

void ind_wifi_sta_off() {
  digitalWrite(pin_wifi_sta, LOW);
}

void xbuzzer_on() {
  unsigned long now = millis();

  if (!buzzerActive) {
    buzzerActive = true;
    xbuzzerState = true;
    buzzerLastChange = now;
    digitalWrite(pin_buzzer, HIGH); // Nyalakan buzzer
    return;
  }

  if (xbuzzerState and (now - buzzerLastChange >= BUZZER_ON_DURATION)) {
    xbuzzerState = false;
    buzzerLastChange = now;
    digitalWrite(pin_buzzer, LOW); // Matikan buzzer
  }
  else if (!xbuzzerState and (now - buzzerLastChange >= BUZZER_OFF_DURATION)) {
    xbuzzerState = true;
    buzzerLastChange = now;
    digitalWrite(pin_buzzer, HIGH); // Nyalakan buzzer
  }
}

void buzzer_stop() {
  buzzerActive = false;
  xbuzzerState = false;
  digitalWrite(pin_buzzer, LOW); // Pastikan buzzer mati
}

void buzzer_beep() {
  digitalWrite(pin_buzzer, HIGH);
  delay(200);
  digitalWrite(pin_buzzer, LOW);
}
