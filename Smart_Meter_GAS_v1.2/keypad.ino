void setup_keypad() {
  Wire.begin();
  if (!pcf.begin()) {
    Serial.println("Gagal inisialisasi PCF8575");
    while (1);
  }
  for (int i = 8; i <= 15; i++) {
    pcf.write(i, HIGH);
  }
}

// Fungsi bantu: tampilkan input kode dengan membersihkan baris LCD
void showDataKey() {
  lcd.setCursor(0, 0);
  lcd.print("Masukan Kode:   ");
  lcd.setCursor(0, 1);
  lcd.print("                ");  // hapus baris kedua
  lcd.setCursor(0, 1);
  lcd.print(data_key);
}

void loop_keypad() {
  key = getKey();

  // ====== SAAT TOMBOL DITEKAN ======
  if (key != '\0') {
    buzzer_beep();
    lastKeyPressTime = millis();

    if (!backlightOn) {
      lcd.backlight();
      backlightOn = true;
    }

    // === Menu Utama: Input angka langsung masuk ke menu 1 ===
    if (en_menu == 0 && key >= '0' && key <= '9') {
      en_menu = 1;
      data_key = String(key);
      menu1StartTime = millis();
      lcd.clear();
      showDataKey();
      return;
    }

    // === Menu 5: OFF Selenoid ===
    if (en_menu == 0 && key == '*') {
      en_menu = 5;
      data_key = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("OFF Selenoid");
      delay(1500);
      lcd.clear();
      showDataKey();
      return;
    }

    // === Menu 6: WIFI Info ===
    if (en_menu == 0 && key == '#') {
      en_menu = 6;
      data_key = "";
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WIFI Info");
      delay(1500);
      lcd.clear();
      showDataKey();
      return;
    }

    // === Menu 0: change slide ===
    if (en_menu == 0 && key == 'B') {
      lcd.clear();
      showMain1 = !showMain1;
      return;
    }

    // === Menu 1, 5, 6: Input Kode ===
    if (en_menu == 1 || en_menu == 5 || en_menu == 6) {
      menu1StartTime = millis();

      if (key >= '0' && key <= '9') {
        data_key += key;
      }
      else if (key == 'B' && data_key.length() > 0) {
        data_key.remove(data_key.length() - 1);
      }
      else if (key == 'C') {
        data_key = "";
      }
      else if (key == 'A') {
        Serial.println("Input dibatalkan. Kembali ke menu utama.");
        data_key = "";
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Dibatalkan");
        delay(1500);
        lcd.clear();
        en_menu = 0;
        return;
      }
      else if (key == 'D') {
        data_key.trim();
        code.trim();
        lcd.clear();

        if (data_key == code) {
          if (en_menu == 1) {
            lcd.print("Selenoid ON Success");
            start_selenoid();
          } else if (en_menu == 5) {
            lcd.print("Selenoid OFF Success");
            stop_selenoid();
          } else if (en_menu == 6) {
            lcd.print("WIFI Info Sukses");
            delay(1000);
            lcd.clear();
            en_menu = 7;
            wifiInfoStartTime = millis();
            wifiInfoDisplayed = false;
            data_key = "";
            return;
          }
          en_menu = 0;
        } else {
          if (en_menu == 1) lcd.print("Selenoid ON Failed");
          else if (en_menu == 5) lcd.print("Selenoid OFF Failed");
          else if (en_menu == 6) lcd.print("WIFI Info Failed");
          en_menu = 0;
        }

        data_key = "";
        delay(1500);
        lcd.clear();
        return;
      }

      // Tampilkan input yang sudah dibersihkan
      showDataKey();
    }

    // === Timeout otomatis menu 1, 5, 6 ===
    if ((en_menu == 1 || en_menu == 5 || en_menu == 6) && millis() - menu1StartTime > 15000) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Timeout. Kembali");
      delay(1500);
      lcd.clear();
      en_menu = 0;
      data_key = "";
    }
  }

  // ====== MENU 7: WIFI INFO (di luar tombol) ======
  if (en_menu == 7) {
    if (key == 'A') {
      Serial.println("Kembali ke menu utama.");
      data_key = "";
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Kembali.");
      delay(1500);
      lcd.clear();
      en_menu = 0;
      return;
    }

    if (!wifiInfoDisplayed) {
      Serial.println("Menampilkan WIFI Info...");
      display_wifi_info();
      wifiInfoDisplayed = true;
    }

    if (millis() - wifiInfoStartTime > 10000) {
      Serial.println("Timeout WIFI Info. Kembali ke menu utama.");
      lcd.clear();
      en_menu = 0;
      wifiInfoDisplayed = false;
    }
  }
}

char getKey() {
  static unsigned long lastDebounceTime = 0;
  static const unsigned long debounceDelay = 50;

  for (byte row = 0; row < ROWS; row++) {
    for (byte r = 0; r < ROWS; r++) {
      pcf.write(rowPins[r], HIGH);
    }

    pcf.write(rowPins[row], LOW);

    for (byte col = 0; col < COLS; col++) {
      if (pcf.read(colPins[col]) == LOW) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
          lastDebounceTime = millis();
          while (pcf.read(colPins[col]) == LOW);
          return keys[row][col];
        }
      }
    }

    pcf.write(rowPins[row], HIGH);
  }

  return '\0';
}

void handleKodeInput(byte menuTarget) {
  menu1StartTime = millis();  // reset timer setiap ada input

  if (key >= '0' and key <= '9') {
    data_key += key;
  }
  else if (key == 'B') {
    if (data_key.length() > 0) data_key.remove(data_key.length() - 1);
  }
  else if (key == 'C') {
    data_key = "";
  }
  else if (key == 'D') {
    data_key.trim();
    code.trim();

    lcd.clear();
    if (data_key == code) {
      switch (menuTarget) {
        case 1:
          lcd.setCursor(0, 0);
          lcd.print("Selenoid ON Success");
          start_selenoid();
          break;
        case 5:
          lcd.setCursor(0, 0);
          lcd.print("Selenoid OFF Success");
          stop_selenoid();
          break;
        case 6:
          lcd.print("WIFI Info Sukses");
          wifiInfoStartTime = millis();
          wifiInfoDisplayed = false;
          lcd.clear();
          en_menu = 7;
          data_key = "";
          delay(1000);
          return;
      }
    } else {
      switch (menuTarget) {
        case 1: lcd.print("Selenoid ON Failed "); break;
        case 5: lcd.print("Selenoid OFF Failed "); break;
        case 6: lcd.print("WIFI Info Failed "); break;
      }
    }
    delay(1500);
    lcd.clear();
    en_menu = 0;
    data_key = "";
    return;
  }
  else if (key == 'A') {
    Serial.println("Input dibatalkan. Kembali ke menu utama.");
    data_key = "";
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Dibatalkan");
    delay(1500);
    lcd.clear();
    en_menu = 0;
    return;
  }

  // Menampilkan input terbaru
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Masukan Kode:");
  lcd.setCursor(0, 1);
  lcd.print(data_key);
}
