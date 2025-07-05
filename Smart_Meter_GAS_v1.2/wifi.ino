void setup_wifi() {
  Serial.println("en_ap: " + en_ap);
  Serial.println("en_client: " + en_client);

  if (en_ap.equals("enable") and en_client.equals("enable")) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_ap.c_str(), password_ap.c_str());
    ind_wifi_ap_on();
    ind_wifi_sta_on();
    WiFi.begin(ssid_client.c_str(), password_client.c_str());
    Serial.println("[WiFi] Mode: AP + STA");
    waitForConnection();

    Serial.print("STA IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    server_begin();

    apStartTime = millis();
    lastClientSeen = millis();
    apActive = true;
  }
  else if (en_ap.equals("enable") and en_client.equals("disable")) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid_ap.c_str(), password_ap.c_str());
    ind_wifi_ap_on();
    ind_wifi_sta_off();

    Serial.println("[WiFi] Mode: AP Only");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    server_begin();

    apStartTime = millis();
    lastClientSeen = millis();
    apActive = true;
  }
  else if (en_ap.equals("disable") and en_client.equals("enable")) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_client.c_str(), password_client.c_str());
    ind_wifi_sta_on();
    ind_wifi_ap_off();

    Serial.println("[WiFi] Mode: STA Only");
    waitForConnection();

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("STA IP Address: ");
      Serial.println(WiFi.localIP());

      server.begin();
      server_begin();
    }
    else {
      Serial.println("[WiFi] STA connection failed. AP fallback or reset may be needed.");
    }
  }
  else {
    Serial.println("[WiFi] Mode not set properly. Both AP and STA are disabled.");
  }
}

void loop_wifi() {
  static unsigned long lastTime;
  if ((millis() - lastTime) > 500) {
    last_update = dateTime;
    String get_payload = payload();
    //    Serial.println("//web_socket_payload_send");
    //    Serial.println(get_payload);
    notifyClients(get_payload);
    lastTime = millis();
  }

  if (en_client == "enable") {
    reconnectWiFiSTA();
  }

  loop_ap();
  ws.cleanupClients();
}

void waitForConnection() {
  Serial.print("Connecting to WiFi...");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED and millis() - start < 10000) {
    display_wifi_connecting();
    Serial.println(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    display_wifi_failed();
    Serial.println("\n[WiFi] Failed to connect.");
  }
}

void reconnectWiFiSTA() {
  static unsigned long lastAttempt = 0;
  static bool wasConnectedBefore = false;
  const unsigned long interval = 10000;

  if (!wifi_reconnect_enabled) {
    Serial.println("[WiFi] Auto-reconnect disabled due to max failed attempts");
    return;
  }

  if ((WiFi.getMode() & WIFI_STA) and WiFi.status() != WL_CONNECTED) {
    unsigned long now = millis();

    if (en_menu != 3) {
      en_menu = 3;  // Tampilkan status reconnecting hanya sekali
      Serial.println("[WiFi] LCD switched to reconnecting screen (en_menu = 3)");
      display_wifi_reconnecting();
    }

    if (now - lastAttempt >= interval) {
      Serial.print("[WiFi] STA disconnected, attempting reconnect #");
      Serial.println(wifi_reconnect_attempts + 1);

      WiFi.disconnect();
      WiFi.begin(ssid_client.c_str(), password_client.c_str());
      lastAttempt = now;
      wifi_reconnect_attempts++;

      if (wifi_reconnect_attempts >= MAX_WIFI_ATTEMPTS) {
        wifi_reconnect_enabled = false;
        Serial.println("❌ Too many failed attempts. Stopping auto-reconnect.");
        en_menu = 0; // Kembali ke tampilan normal
      }
    }

    wasConnectedBefore = false;
  }

  if (WiFi.status() == WL_CONNECTED and !wasConnectedBefore) {
    wasConnectedBefore = true;
    wifi_reconnect_attempts = 0;
    wifi_reconnect_enabled = true;

    Serial.println("[WiFi] STA reconnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    en_menu = 0;  // Kembali ke tampilan normal
  }

  if (WiFi.isConnected()) {
    ssid    = WiFi.SSID();
    ip      = WiFi.localIP().toString();
    mac     = WiFi.macAddress();
    gateway = WiFi.gatewayIP().toString();
    subnet  = WiFi.subnetMask().toString();
    rssi    = WiFi.RSSI();
  }
}

void loop_ap() {
  if (en_menu != 4) return;

  int adcValue = ads.readADC_SingleEnded(3);
  bool button = (adcValue > 500);

  static bool longPressHandled = false;

  if (!button) {
    if (buttonPressed && !longPressHandled) {
      Serial.println("Tombol dilepas sebelum 5 detik. Kembali ke menu utama.");
      lcd.clear();
      en_menu = 0;
    }

    buttonPressed = false;
    longPressHandled = false;
    lastCountdown = -1;
  }

  if (button) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStart = millis();
      longPressHandled = false;
    }
    else if (!longPressHandled && millis() - buttonPressStart >= LONG_PRESS_DURATION) {
      if (apActive) {
        Serial.println("Long press detected! Turning AP OFF...");
        ind_wifi_ap_off();
        display_ap_off();
        turnOffAP();
        apActive = false;
      } else {
        Serial.println("Long press detected! Turning AP ON...");
        ind_wifi_ap_on();
        display_ap_on();
        startAccessPoint();
        apActive = true;
        apStartTime = millis();
        lastClientSeen = apStartTime;
      }
      longPressHandled = true;
    }
  }

  // Auto-off logic jika AP aktif
  if (apActive) {
    static int previousClients = 0;
    int clients = WiFi.softAPgetStationNum();

    // Pantau perubahan klien
    if (clients != previousClients) {
      Serial.print("Perubahan jumlah klien: ");
      Serial.println(clients);
      if (clients > 0) {
        lastClientSeen = millis();
      }
      previousClients = clients;
    }

    unsigned long now = millis();

    if (clients == 0) {
      // Tidak pernah konek
      if ((now - apStartTime >= 300000) && (lastClientSeen == apStartTime)) {
        Serial.println("No clients ever connected. Turning off AP...");
        ind_wifi_ap_off();
        display_ap_off();
        turnOffAP();
        apActive = false;
      }
      // Sudah konek, tapi sudah lama tidak ada klien
      else if ((now - lastClientSeen) >= 300000) {
        Serial.println("No clients connected for 5 minutes. Turning off AP...");
        ind_wifi_ap_off();
        display_ap_off();
        turnOffAP();
        apActive = false;
      }
    }
  }
}

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_ap.c_str(), password_ap.c_str());
  Serial.println("AP started...");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  apStartTime = millis();
  lastClientSeen = millis();
  apActive = true;
}

void turnOffAP() {
  WiFi.softAPdisconnect(true);
  apActive = false;
  Serial.println("AP disabled.");
}
