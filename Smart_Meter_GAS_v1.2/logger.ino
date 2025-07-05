void appendLog(float meter, float remaining, float flowrate, float usage_hour, float usage_today) {
  char filename[30];
  sprintf(filename, "/log%04d%02d%02d.txt", Year, Month, Day);

  DynamicJsonDocument doc(1024);

  if (SPIFFS.exists(filename)) {
    File file = SPIFFS.open(filename, "r");
    if (file) {
      deserializeJson(doc, file);
      file.close();
    }
  }
  else {
    doc["log_header"] = "datetime, meter, remaining, flowrate, usage_hour, usage_today";
  }

  char datetime[25];
  sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d", Year, Month, Day, Hour, Minute, Second);

  char logValue[128];
  sprintf(logValue, "%s, %.2f, %.2f, %.2f, %.2f, %.2f",
          datetime, meter, remaining, flowrate, usage_hour, usage_today);

  doc["log1"] = logValue;

  File file = SPIFFS.open(filename, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
  }
}

void cleanupOldLogs() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  struct LogEntry {
    String name;
    uint32_t dateInt;
  } logFiles[40];

  int count = 0;

  while (file) {
    String name = file.name();
    if (name.startsWith("/log") and name.endsWith(".txt")) {
      String dateStr = name.substring(4, name.length() - 4);
      uint32_t dateVal = dateStr.toInt();
      logFiles[count++] = { name, dateVal };
    }
    file = root.openNextFile();
  }

  if (count > 31) {
    for (int i = 0; i < count - 1; i++) {
      for (int j = i + 1; j < count; j++) {
        if (logFiles[i].dateInt > logFiles[j].dateInt) {
          LogEntry temp = logFiles[i];
          logFiles[i] = logFiles[j];
          logFiles[j] = temp;
        }
      }
    }

    for (int i = 0; i < count - 31; i++) {
      Serial.println("Menghapus file: " + logFiles[i].name);
      SPIFFS.remove(logFiles[i].name);
    }
  }
}

String buildCombinedLogJson() {
  DynamicJsonDocument outputDoc(8192);
  int logIndex = 1;

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file and logIndex <= 31) {
    String filename = file.name();

    if (filename.startsWith("/log") and filename.endsWith(".txt")) {
      File logFile = SPIFFS.open(filename, "r");
      if (logFile) {
        DynamicJsonDocument fileDoc(1024);
        DeserializationError err = deserializeJson(fileDoc, logFile);
        logFile.close();

        if (!err) {
          // Ambil log_header hanya dari file yang punya
          if (fileDoc.containsKey("log_header") and !outputDoc.containsKey("log_header")) {
            outputDoc["log_header"] = fileDoc["log_header"];
          }

          // Ambil semua key yang dimulai dengan "log" (log1, log2, dst)
          for (JsonPair kv : fileDoc.as<JsonObject>()) {
            const char* key = kv.key().c_str();
            if (strncmp(key, "log", 3) == 0 and strcmp(key, "log_header") != 0) {
              String newKey = "log" + String(logIndex++);
              outputDoc[newKey] = kv.value();
              if (logIndex > 31) break;
            }
          }
        }

      }
    }

    file = root.openNextFile();
  }

  String jsonStr;
  serializeJson(outputDoc, jsonStr);
  return jsonStr;
}

void loop_logger() {
  static unsigned long lastLogTime = 0;
  static int lastLoggedDay = -1;

  if ((millis() - lastLogTime) >= 10000) {
    lastLogTime = millis();
    appendLog(
      meter.toFloat(),
      remaining.toFloat(),
      flowRateM3h,
      usage_hour,
      usage_today
    );
  }

  if (Day != lastLoggedDay) {
    lastLoggedDay = Day;
    cleanupOldLogs();
  }
}
