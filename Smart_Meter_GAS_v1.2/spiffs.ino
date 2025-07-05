void data_default() // kalau masih  perawan, belum ada database // ntar lu isi defaultnya apa
{
  writeFile(SPIFFS, "/code.txt", "123456");
  writeFile(SPIFFS, "/username.txt", "admin");
  writeFile(SPIFFS, "/password.txt", "admin");
  writeFile(SPIFFS, "/en_client.txt", "disable");
  writeFile(SPIFFS, "/ssid_client.txt", "Smart Meter GAS");
  writeFile(SPIFFS, "/password_client.txt", "12345678");
  writeFile(SPIFFS, "/en_ap.txt", "enable");
  ssid_ap = readFile(SPIFFS, "/serial_number.txt");
  writeFile(SPIFFS, "/ssid_ap.txt", ssid_ap.c_str());
  writeFile(SPIFFS, "/password_ap.txt", "12345678");
  writeFile(SPIFFS, "/meter.txt", "0");
  writeFile(SPIFFS, "/remaining_input.txt", "0");
  writeFile(SPIFFS, "/en_alarm.txt", "disable");
  writeFile(SPIFFS, "/alarm_threshold_min.txt", "10");
  writeFile(SPIFFS, "/en_valve.txt", "disable");
  writeFile(SPIFFS, "/valve_threshold_min.txt", "0");
  writeFile(SPIFFS, "/url_ota.txt", "ota-example.com");
  writeFile(SPIFFS, "/en_hourly_usage.txt", "true");
  writeFile(SPIFFS, "/en_today_usage.txt", "true");
  writeFile(SPIFFS, "/remaining_meter.txt", "0");
  Serial.println("default ok");
}

void read_setting()
{
  username = readFile(SPIFFS, "/username.txt");
  password = readFile(SPIFFS, "/password.txt");

  if (username == "" and password == "")
  {
    data_default();
    delay(200);
  }

  username = readFile(SPIFFS, "/username.txt");
  password = readFile(SPIFFS, "/password.txt");
  en_client = readFile(SPIFFS, "/en_client.txt");
  ssid_client =  readFile(SPIFFS, "/ssid_client.txt");
  password_client =  readFile(SPIFFS, "/password_client.txt");
  en_ap = readFile(SPIFFS, "/en_ap.txt");
  ssid_ap =  readFile(SPIFFS, "/ssid_ap.txt");
  password_ap =  readFile(SPIFFS, "/password_ap.txt");
  meter = readFile(SPIFFS, "/meter.txt");
  remaining_input = readFile(SPIFFS, "/remaining_input.txt");
  en_alarm = readFile(SPIFFS, "/en_alarm.txt");
  alarm_threshold_min = readFile(SPIFFS, "/alarm_threshold_min.txt");
  en_valve = readFile(SPIFFS, "/en_valve.txt");
  valve_threshold_min = readFile(SPIFFS, "/valve_threshold_min.txt");
  serial_number = readFile(SPIFFS, "/serial_number.txt");
  serial_number.trim();
  assembly_date = readFile(SPIFFS, "/assembly_date.txt");
  code = readFile(SPIFFS, "/code.txt");
  url_ota = readFile(SPIFFS, "/url_ota.txt");
  en_hourly_usage = readFile(SPIFFS, "/en_hourly_usage.txt");
  datetime_start_hourly_usage = readFile(SPIFFS, "/datetime_start_hourly.txt");
  meter_start_hourly_usage = readFile(SPIFFS, "/meter_start_hourly_usage.txt");
  en_today_usage = readFile(SPIFFS, "/en_today_usage.txt");
  datetime_start_today_usage = readFile(SPIFFS, "/datetime_start_today.txt");
  meter_start_today_usage = readFile(SPIFFS, "/meter_start_today_usage.txt");
  remaining_meter = readFile(SPIFFS, "/remaining_meter.txt");
  meter_dm = meter.toInt();
  xmeter = meter;

  Serial.print("SPIFFS Free: "); Serial.println(humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  Serial.print("SPIFFS Used: "); Serial.println(humanReadableSize(SPIFFS.usedBytes()));
  Serial.print("SPIFFS Total: "); Serial.println(humanReadableSize(SPIFFS.totalBytes()));
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String readFile(fs::FS &fs, const char * path) {
  File file = fs.open(path, "r");
  if (!file or file.isDirectory()) {
    Serial.print("⚠️ Gagal membaca file: ");
    Serial.println(path);
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent += (char)file.read();
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, "w");
  if (!file) {
    Serial.print("⚠️ Gagal menulis file: ");
    Serial.println(path);
    return;
  }

  file.print(message);
  file.close();
}

void setup_webserver() {
  ssid = default_ssid;
  wifipassword = default_wifipassword;
  httpuser = default_httpuser;
  httppassword = default_httppassword;
  webserverporthttp = default_webserverporthttp;
  configureWebServer();
  Serial.println("Starting Webserver ...");
}

void configureWebServer() {
  server.onNotFound(notFound);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();

    if (checkUserWebAuth(request)) {
      logmessage += " Auth: Success";
      Serial.println(logmessage);
      request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
    }
    else {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }
  });
}

void notFound(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);
  request->send(404, "text/plain", "Not found");
}

bool checkUserWebAuth(AsyncWebServerRequest * request) {
  bool isAuthenticated = false;
  if (request->authenticate(username.c_str(), password.c_str())) {
    Serial.println("is authenticated via username and password");
    isAuthenticated = true;
  }
  return isAuthenticated;
}

void notFoundd(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

String getSelected(fs::FS &fs, const char * path, bool enableCheck) {
  String x = readFile(fs, path);
  if (enableCheck) {
    return (x == "enable") ? "selected" : "";
  } else {
    return (x == "disable") ? "selected" : "";
  }
}

String processor(const String& var) {
  if (var == "meter") return meter;
  if (var == "remaining") return remaining_input;
  if (var == "alarm_threshold_min") return String(alarm_threshold_min.toFloat() / 0.01);
  if (var == "valve_threshold_min") return String(valve_threshold_min.toFloat() / 0.01);
  if (var == "date") return dates;
  if (var == "time") return times;
  if (var == "ssid_client") return ssid_client;
  if (var == "password_client") return password_client;
  if (var == "ssid_ap") return ssid_ap;
  if (var == "password_ap") return password_ap;
  if (var == "username") return username;
  if (var == "password") return password;
  if (var == "url_ota") return url_ota;

  if (var == "selected_enable_alarm")   return getSelected(SPIFFS, "/en_alarm.txt", true);
  if (var == "selected_disable_alarm")  return getSelected(SPIFFS, "/en_alarm.txt", false);

  if (var == "selected_enable_valve")   return getSelected(SPIFFS, "/en_valve.txt", true);
  if (var == "selected_disable_valve")  return getSelected(SPIFFS, "/en_valve.txt", false);

  if (var == "selected_enable_client")  return getSelected(SPIFFS, "/en_client.txt", true);
  if (var == "selected_disable_client") return getSelected(SPIFFS, "/en_client.txt", false);

  if (var == "selected_enable_ap")      return getSelected(SPIFFS, "/en_ap.txt", true);
  if (var == "selected_disable_ap")     return getSelected(SPIFFS, "/en_ap.txt", false);

  if (var == "selected_enable_ap") {
    String x = readFile(SPIFFS, "/en_ap.txt");
    Serial.println("[AP ENABLE] Value: " + x);
    return getSelected(SPIFFS, "/en_ap.txt", true);
  }
  if (var == "selected_disable_ap") {
    String x = readFile(SPIFFS, "/en_ap.txt");
    Serial.println("[AP DISABLE] Value: " + x);
    return getSelected(SPIFFS, "/en_ap.txt", false);
  }


  return String();
}

void server_begin() {
  server.serveStatic("/", SPIFFS, "/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();

    if (!request->authenticate(username.c_str(), password.c_str())) {
      logmessage += " Auth: Failed";
      Serial.println(logmessage);
      return request->requestAuthentication();
    }

    logmessage += " Auth: Success";
    Serial.println(logmessage);
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/logged-out", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(401);
  });

  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(401, "text/plain", "Logged out");
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(username.c_str(), password.c_str()))
      return request->requestAuthentication();

    Serial.println("================");
    Serial.println("Device Restart");
    Serial.println("================");
    request->send(200, "text/plain", "Restarting...");
    delay(3000);
    ESP.restart();
  });

  server.on("/default", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(username.c_str(), password.c_str()))
      return request->requestAuthentication();

    Serial.println("================");
    Serial.println("Device Reset Default");
    Serial.println("================");
    data_default();  // Reset default config
    request->send(200, "text/plain", "Resetting to default...");
    delay(3000);
    ESP.restart();
  });

  server.on("/publish", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(username.c_str(), password.c_str()))
      return request->requestAuthentication();

    Serial.println();
    Serial.println("__________________MQTT TEST_______________________");
    Serial.println();
    request->send(SPIFFS, "/dashboard_ws.html", String(), false, processor);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(username.c_str(), password.c_str()))
      return request->requestAuthentication();

    request->redirect("/");
  });

  server.onNotFound(notFound);
  server.begin();
}
