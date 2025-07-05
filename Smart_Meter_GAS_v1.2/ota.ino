//fmw.ppa-bib.net/towerlamp/v3.9.bin
void ota() {
  Serial.print("Checking WiFi connection...");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" Not connected.");
    return;
  }
  Serial.println(" Connected.");

  Serial.println("Triggering OTA from URL: " + url_ota);

  HTTPClient http;
  http.begin(url_ota);

  // Configure HTTP client
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.addHeader("Cache-Control", "no-cache");

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
    if (httpCode > 0) {
      Serial.println(http.getString());
    }
    http.end();
    return;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("Invalid content length");
    http.end();
    return;
  }

  Serial.printf("Firmware size: %.2f MB\n", contentLength / 1024.0 / 1024.0);

  if (!Update.begin(contentLength, U_FLASH)) {
    Serial.printf("Not enough space for OTA update. Needed: %d\n", contentLength);
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();
  uint8_t buff[1024] = {0};
  size_t written = 0;
  unsigned long lastProgressUpdate = 0;
  int lastPercent = -1;

  Serial.println("Starting update...");

  while (http.connected() and (written < contentLength)) {
    size_t available = stream->available();
    if (available) {
      int bytesRead = stream->readBytes(buff, min(available, sizeof(buff)));
      if (bytesRead > 0) {
        size_t bytesWritten = Update.write(buff, bytesRead);
        if (bytesWritten != bytesRead) {
          Serial.printf("Write failed. Expected: %d, Actual: %d\n", bytesRead, bytesWritten);
          http.end();
          Update.abort();
          return;
        }
        written += bytesWritten;

        // Update progress only when percentage changes or every 2 seconds
        int currentPercent = (written * 100) / contentLength;
        unsigned long now = millis();
        if ((currentPercent != lastPercent) or (now - lastProgressUpdate > 2000)) {
          Serial.printf("Progress: %d%% (%d/%d bytes)\n", currentPercent, written, contentLength);
          lastPercent = currentPercent;
          lastProgressUpdate = now;
        }
        esp_task_wdt_reset(); // Feed the watchdog
      }
    }
    delay(1);
  }

  http.end();

  if (written != contentLength) {
    Serial.printf("Download incomplete. Expected: %d, Received: %d\n", contentLength, written);
    Update.abort();
    return;
  }

  if (Update.end()) {
    if (Update.isFinished()) {
      Serial.println("OTA update successful, restarting in 3 seconds...");
      Serial.flush();
      delay(3000);
      ESP.restart();
    } else {
      Serial.println("OTA update failed: not finished.");
    }
  } else {
    Serial.printf("OTA update error: %s\n", Update.errorString());
    Update.abort();
  }
}
