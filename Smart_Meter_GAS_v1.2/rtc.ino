void setup_rtc() {
  Wire.begin();
  Rtc.Begin();

  if (!Rtc.IsDateTimeValid()) {
    if (Rtc.LastError() != 0) {
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    } else {
      Serial.println("RTC tidak valid: Baterai lemah atau belum disetel.");
    }
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC tidak berjalan, menyetelnya agar berjalan...");
    Rtc.SetIsRunning(true);
  }
}

void loop_rtc() {
  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);

  Year   = now.Year();
  Month  = now.Month();
  Day    = now.Day();
  Hour   = now.Hour();
  Minute = now.Minute();
  Second = now.Second();

  RtcTemperature temp = Rtc.GetTemperature();
  board_temp = String(temp.AsFloatDegC(), 1);
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime& dt) {
  char tanggalx[16];
  char waktux[16];

  snprintf_P(tanggalx, countof(tanggalx), PSTR("%04u-%02u-%02u"),
             dt.Year(), dt.Month(), dt.Day());

  snprintf_P(waktux, countof(waktux), PSTR("%02u:%02u:%02u"),
             dt.Hour(), dt.Minute(), dt.Second());

  dates = String(tanggalx);
  times = String(waktux);
  dateTime = dates + " " + times;
}
