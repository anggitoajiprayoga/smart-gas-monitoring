#include <esp_task_wdt.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "FS.h"
#include <ArduinoJson.h>
#include <Update.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include <AsyncElegantOTA.h>
#include <HTTPClient.h>
#include <Adafruit_ADS1X15.h>
#include <PCF8575.h>

#define WDT_TIMEOUT 60
#define debug false

#define BUTTON_PIN 0
#define LONG_PRESS_DURATION 5000
#define GAS_PER_PULSE_M3  0.01
#define FLOW_INTERVAL_MS     60000

#define PULSE_PIN 14
#define pin_proses 25
#define pin_buzzer 26
#define pin_ind_selenoid 32
#define pin_wifi_sta 13
#define pin_wifi_ap 33
#define pin_selenoid 4
#define HOLD_TIME   5000

const byte ROWS = 4;
const byte COLS = 4;

byte rowPins[ROWS] = {8, 9, 10, 11};
byte colPins[COLS] = {12, 13, 14, 15};

char keys[ROWS][COLS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};
char keyPressed = '\0';

volatile unsigned long pulseCount = 0;
volatile unsigned long meter_dm = 0;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 500;

String board_version = "Smart Meter Gas v1";
String firmware_version = "v1.1";
String serial_number = "SMG-01-001";
String assembly_date = "16-06-2025";

String username, password, en_client, ssid_client, password_client,
       en_ap, ssid_ap, password_ap, xmeter, meter, remaining, remaining_meter,
       en_alarm, alarm_threshold_min, en_valve, valve_threshold_min, remaining_input;

String freesp, usedstr, totalstr;

String webpage = "";
bool shouldReboot = false;
String listFiles(bool ishtml = false);

const String default_ssid = "";
const String default_wifipassword = "SS6";
const String default_httpuser = "admin";
const String default_httppassword = "admin";
const int default_webserverporthttp = 80;

String ssid;
String wifipassword;
String httpuser;
String httppassword;
int webserverporthttp;

uint64_t sizeTotal, sizeKB, totalKB, usedKB, usedB, spaceKB;
uint32_t usedByte;

double meter_final, remaining_gas, usage_hour, usage_today;
int rssi, battery_percent;
float battery_voltage;
String ip, mac, gateway, subnet, last_update, url_ota;
String dates, times, dateTime, board_temp, code, setDateStr, setTimeStr;
int Year, Month, Day, Hour, Minute, Second;
String en_hourly_usage, datetime_start_hourly_usage, meter_start_hourly_usage;
String en_today_usage, datetime_start_today_usage, meter_start_today_usage;

unsigned long apStartTime = 0;
unsigned long lastClientSeen = 0;
bool apActive = false;

bool buttonPressed = false;
unsigned long buttonPressStart = 0;
unsigned long lastCalcTime = 0;
float flowRateM3h = 0.0;
int lastDay = -1;
char key;
byte en_menu = 0;
int menu;
String data_key;

unsigned long lastSwitch = 0;

int16_t adc0, adc1, adc2, adc3;
float volts0, volts1, volts2, volts3;

enum BuzzerMode {
  BUZZ_OFF,
  BUZZ_SHORT,
  BUZZ_LONG,
  BUZZ_TRIPLE,
  BUZZ_DOUBLE
};
BuzzerMode buzzerMode = BUZZ_OFF;

unsigned long buzzerTimer = 0, ledOnTime;
int buzzerStep = 0;
bool buzzerState = false, ledState, pcfReady = false;
unsigned long lastKeyPressTime = 0;
const unsigned long backlightTimeout = 30000; // 30 detik
bool backlightOn = true;
bool lastButtonState = false;
bool alarm_on = false;
long prev_buz_on, prev_buz_off;
bool buzzer_on, buzzer_off, interlock_alarm = true, button_pause;
unsigned long alarm_start_time = 0;
unsigned long alarm_snooze_start = 0;

const unsigned long ALARM_DURATION = 60000;           // 1 menit
const unsigned long ALARM_SNOOZE_DURATION = 600000;   // 10 menit

bool buzzerActive = false;
bool xbuzzerState = false;
unsigned long buzzerLastChange = 0;
const unsigned long BUZZER_ON_DURATION = 1000;    // 1 detik nyala
const unsigned long BUZZER_OFF_DURATION = 3000;   // 3 detik mati

bool beepActive = false;
unsigned long beepStartTime = 0;
const unsigned long BEEP_DURATION = 200;  // Durasi beep 200 ms

bool wifi_reconnect_enabled = true;
int wifi_reconnect_attempts = 0;
const int MAX_WIFI_ATTEMPTS = 5;

unsigned long lastDisplaySwitch = 0;
const unsigned long displayInterval = 5000;  // Ganti tampilan tiap 5 detik
bool showMain1 = true;

float usage_hour_fixed = 0;
bool usage_hour_is_fixed = false;

bool ap_button_mode = false;           // Menandakan sedang di mode aktifkan AP
unsigned long ap_button_mode_timer = 0;
const unsigned long AP_BUTTON_MODE_TIMEOUT = 5000;  // kembali ke menu utama setelah 5 detik
unsigned long menu1StartTime = 0;
unsigned long wifiInfoStartTime = 0;
bool wifiInfoDisplayed = false;

volatile bool pulseFlag = false;
volatile unsigned long interruptTime = 0;

portMUX_TYPE pulseMux = portMUX_INITIALIZER_UNLOCKED;
TaskHandle_t TaskBuzzerHandle;
PCF8575 pcf(0x20);
Adafruit_ADS1015 ads;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiServer serverAP(80);
RtcDS3231<TwoWire> Rtc(Wire);
LiquidCrystal_I2C lcd(0x27, 20, 4);

String payload() {
  String data = "";
  DynamicJsonDocument buf(1024);
  JsonObject obj = buf.to<JsonObject>();
  obj["meter_reading"]   = meter_final;
  obj["remaining_volume"] = String(remaining_gas, 2);
  obj["flow_rate"]       = String(flowRateM3h, 2);
  obj["battery_voltage"] = String(battery_voltage, 1);
  obj["battery_percent"] = battery_percent;
  obj["board_temp"]      = board_temp;
  obj["usage_hour"]      = String(usage_hour, 2);
  obj["usage_today"]     = String(usage_today, 2);
  obj["ssid"]            = ssid;
  obj["ip"]              = ip;
  obj["mac"]             = mac;
  obj["gateway"]         = gateway;
  obj["subnet"]          = subnet;
  obj["rssi"]            = rssi;
  obj["board_ver"]       = board_version;
  obj["firm_ver"]        = firmware_version;
  obj["sn"]              = serial_number;
  obj["assembly_date"]   = assembly_date;
  obj["date_time"]       = dateTime;
  obj["last_update"]     = last_update;

  static unsigned long prev_mcu;
  if ((millis() - prev_mcu) > 5000) {
    obj["chip_model"]    = String(ESP.getChipModel());
    obj["chip_revision"] = ESP.getChipRevision();
    obj["chip_cores"]    = ESP.getChipCores();
    obj["cpu_freq"]      = ESP.getCpuFreqMHz();
    obj["flash_size"]    = (int)(ESP.getFlashChipSize() / 1024);
    obj["heap_free"]     = ESP.getFreeHeap();
    obj["heap_min"]      = ESP.getMinFreeHeap();
    obj["sketch_size"]   = ESP.getSketchSize();
    obj["sketch_free"]   = ESP.getFreeSketchSpace();
    prev_mcu = millis();
  }

  serializeJson(obj, data);
  return data;
}

void notifyClients(String payload) {
  ws.textAll(payload);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final and info->index == 0 and info->len == len and info->opcode == WS_TEXT) {
    data[len] = 0;
    Serial.println((char*)data);
    parse_websocket((char*)data);
    String get_payload = payload();
    //    Serial.println("//web_socket_payload_receive//");
    //    Serial.println(get_payload);
    notifyClients(get_payload);

    if (String((char*)data) == "restart") {
      Serial.println("Restart Device by Web Server");
      ESP.restart();
    }
    else if (String((char*)data) == "reset_default") {
      Serial.println("Restart Default by Web server");
      data_default();
      delay(500);
      read_setting();
    }
    else if (String((char*)data) == "show_log") {
      Serial.println("Send Log");
      String fullLog = buildCombinedLogJson();
      notifyClients(fullLog);
      Serial.println(fullLog);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
      Serial.printf("WebSocket client #%u pong received\n", client->id());
      break;
    case WS_EVT_ERROR:
      Serial
      .println("WebSocket error occurred. Restarting ESP32...");
      delay(2000);
      ESP.restart();
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void IRAM_ATTR onPulse() {
  unsigned long now = micros();
  static unsigned long lastInterruptTime = 0;

  if (now - lastInterruptTime > debounceDelay * 300UL) {
    portENTER_CRITICAL_ISR(&pulseMux);
    interruptTime = now;
    pulseFlag = true;
    portEXIT_CRITICAL_ISR(&pulseMux);

    lastInterruptTime = now;
  }
}

//void IRAM_ATTR onPulse() {
//  unsigned long currentTime = millis();
//  if (currentTime - lastInterruptTime > debounceDelay) {
//    pulseCount++;
//    meter_dm++;
//    meter = String(meter_dm);
//    lastInterruptTime = currentTime;
//    digitalWrite(pin_proses, HIGH);
//    ledOnTime = currentTime;
//    ledState = true;
//  }
//}

void TaskBuzzer(void *parameter) {
  while (true) {
    if (alarm_on == true and interlock_alarm == true) {
      if (buzzer_on == false and buzzer_off == false) {
        buzzer_on = true;
        buzzer_off = false;
        prev_buz_on = millis();
      }
      if (buzzer_on == true and buzzer_off == false) {
        if ((millis() - prev_buz_on) > ALARM_DURATION) {
          buzzer_on = false;
          buzzer_off = true;
          prev_buz_off = millis();
        }
      }
      if (buzzer_on == false and buzzer_off == true) {
        if ((millis() - prev_buz_off) > ALARM_SNOOZE_DURATION) {
          buzzer_on = true;
          buzzer_off = false;
          prev_buz_on = millis();
        }
      }
      if (buzzer_on == true) {
        xbuzzer_on();
      }
      else {
        buzzer_stop();
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Delay kecil (non-blocking)
  }
}

void printResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("Reset Reason: %d\n", reason);
}

void setup() {
  if (debug) {
    Serial.begin(115200);
  }
  printResetReason();
  setup_main();
  SPIFFS.begin();
  ads.begin();
  read_setting();
  setup_display();
  waitForResetButton();
  setup_webserver();
  AsyncElegantOTA.begin(&server);
  dispay_close_intro();
  setup_wifi();
  initWebSocket();
  setup_rtc();
  setup_sensor();
  cleanupOldLogs();
  xTaskCreatePinnedToCore(TaskBuzzer, "TaskBuzzer", 1000, NULL, 1, &TaskBuzzerHandle, 1);
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  lcd.clear();
}

void loop() {
  loop_keypad();
  loop_rtc();
  loop_ads();
  if (en_menu == 0) {
    loop_sensor();
  }
  loop_main();
  loop_wifi();
  loop_logger();
  if (debug) {
    serial_monitor();
  }
  esp_task_wdt_reset();
}
