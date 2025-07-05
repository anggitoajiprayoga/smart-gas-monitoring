void setup_display() {
  lcd.init();
  lcd.backlight();
  display_intro();
  lastKeyPressTime = millis();  // Mulai timer saat dinyalakan
}

void display_intro() {
  lcd.clear();

  String line1 = "  SMART METER GAS   ";
  String line2 = "     " + serial_number + "     ";
  String line3 = "    Version 1.0     ";
  String line4 = "  Initializing...   ";

  for (int i = 0; i < line1.length(); i++) {
    lcd.setCursor(i, 0);
    lcd.print(line1[i]);
    delay(60);
  }
  for (int i = 0; i < line2.length(); i++) {
    lcd.setCursor(i, 1);
    lcd.print(line2[i]);
    delay(60);
  }
  for (int i = 0; i < line3.length(); i++) {
    lcd.setCursor(i, 2);
    lcd.print(line3[i]);
    delay(60);
  }

  lcd.setCursor(2, 3);
  lcd.print("Initializing");
  for (int i = 0; i < 3; i++) {
    lcd.print(".");
    delay(400);
  }
}

void dispay_close_intro() {
  lcd.setCursor(0, 3);
  lcd.print("   Ready to Use    ");
}

void dispay_main1() {
  lcd.setCursor(0, 0);
  lcd.print("G.M3    :" + String(meter_final, 2) + "m3");
  lcd.setCursor(0, 1);
  lcd.print("Rmnng   :" + String(remaining_gas, 2) + "m3");
  lcd.setCursor(0, 2);
  lcd.print("Use Hour:" + String(usage_hour, 2) + "m3");
  lcd.setCursor(0, 3);
  lcd.print("Use Day :" + String(usage_today, 2) + "m3");
}

void dispay_main2() {
  lcd.setCursor(0, 0);
  lcd.print("Brd Tmp:" + board_temp + (char)223 + "C");
  lcd.setCursor(0, 1);
  lcd.print("Batt   :" + String(battery_percent) + "%");
  lcd.setCursor(0, 2);
  lcd.print("Date   :" + dates);
  lcd.setCursor(0, 3);
  lcd.print("Time   :" + times);
}

void display_wifi_info() {
  ssid_ap.trim();
  password_ap.trim();
  ssid_client.trim();
  password_client.trim();
  lcd.setCursor(0, 0);
  lcd.print("AP : " + ssid_ap);
  lcd.setCursor(0, 1);
  lcd.print("PW : " + password_ap);
  lcd.setCursor(0, 2);
  lcd.print("STA: " + ssid_client);
  lcd.setCursor(0, 3);
  lcd.print("PW : " + password_client);
}

void display_ap_on() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi AP Mode Aktif  ");
  lcd.setCursor(0, 1);
  lcd.print("SSID : " + ssid_ap + "   ");  // Ganti sesuai variabel SSID Anda
  lcd.setCursor(0, 2);
  lcd.print("IP: 192.168.4.1     ");      // Ganti jika perlu tampilkan IP AP
  lcd.setCursor(0, 3);
  lcd.print("Menunggu Koneksi... ");
  delay(3000);
  lcd.clear();
}

void gas_habis() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("      GAS HABIS      ");
  lcd.setCursor(0, 1);
  lcd.print("     Selenoid OFF    ");
  lcd.setCursor(0, 2);
  lcd.print("                     ");
  lcd.setCursor(0, 3);
  lcd.print("                     ");
  delay(2000);
  lcd.clear();
}

void display_ap_toggle_hint() {
  lcd.setCursor(0, 0);
  lcd.print("Tekan & Tahan Tombol");
  lcd.setCursor(0, 1);
  lcd.print("5 Dtk untuk WiFi AP ");
  lcd.setCursor(0, 2);
  lcd.print(" ON / OFF           ");
  lcd.setCursor(0, 3);
  lcd.print("Lepas utk batalkan  ");
}

void display_ap_off() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi AP Dimatikan   ");
  lcd.setCursor(0, 1);
  lcd.print("Tidak ada koneksi   ");
  lcd.setCursor(0, 2);
  lcd.print("selama 5 menit.     ");
  lcd.setCursor(0, 3);
  lcd.print("Mode hemat daya...  ");
  delay(3000);
  lcd.clear();
}

void display_wifi_reconnecting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Terputus...    ");
  lcd.setCursor(0, 1);
  lcd.print("Coba sambung ulang  ");
  lcd.setCursor(0, 2);
  lcd.print("Tunggu beberapa saat");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
}

void display_wifi_connecting() {
  static int dotCount = 0;
  lcd.setCursor(0, 0);
  lcd.print("Menyambung WiFi...  ");
  lcd.setCursor(0, 1);
  lcd.print("Tunggu sebentar     ");
  lcd.setCursor(0, 2);
  lcd.print("Menghubungkan       ");

  lcd.setCursor(15, 2);
  for (int i = 0; i < (dotCount % 4); i++) {
    lcd.print(".");
  }
  lcd.setCursor(0, 3);
  lcd.print("                ");

  dotCount++;
}

void display_reset() {
  lcd.setCursor(0, 0);
  lcd.print("   RESETTING UNIT   ");
  lcd.setCursor(0, 1);
  lcd.print("   Please wait...   ");
  lcd.setCursor(0, 2);
  lcd.print("     . . . . .      ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
}

void display_button_reset(int countdown) {
  lcd.setCursor(0, 0);
  lcd.print("  Tekan & Tahan     ");
  lcd.setCursor(0, 1);
  lcd.print(" Reset dalam: " + String(countdown) + "s   ");
  lcd.setCursor(0, 2);
  lcd.print(" Lepas utk batalkan ");
  lcd.setCursor(0, 3);
  lcd.print("    . . . . .       ");
}

void display_wifi_failed() {
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print("WIFI Not Connect    ");
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  delay(2000);
  lcd.clear();
}
