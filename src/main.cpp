#include <Adafruit_SHT31.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <PsychicHttp.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFi.h>
#include "arduino_secrets.h"
#include "JosefinSans-18Bold.h"
#include "JosefinSans9.h"

#define EPD_CS   5   // D3
#define EPD_DC   4   // D2
#define EPD_RST  3   // D1
#define EPD_BUSY 21  // D6

#define HOSTNAME  "air-sensor"
#define DEVICE_ID "air_sensor"  // HOSTNAME with hyphens → underscores; change both together
#define MQTT_PORT 1883

static const unsigned long UPDATE_INTERVAL_MS     = 30000;
static const unsigned long WIFI_CHECK_INTERVAL_MS = 30000;
static const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

// Home Assistant MQTT discovery + state topics — all derived from HOSTNAME/DEVICE_ID
static const char* DISCO_TEMP  = "homeassistant/sensor/" HOSTNAME "/temperature/config";
static const char* DISCO_HUMID = "homeassistant/sensor/" HOSTNAME "/humidity/config";
static const char* STATE_TEMP  = HOSTNAME "/sensor/temperature/state";
static const char* STATE_HUMID = HOSTNAME "/sensor/humidity/state";

Adafruit_SHT31    sht30;
PsychicHttpServer server;
WiFiClient        wifiClient;
PubSubClient      mqtt(wifiClient);
GxEPD2_BW<GxEPD2_290_GDEY029T94, GxEPD2_290_GDEY029T94::HEIGHT> epd(GxEPD2_290_GDEY029T94(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

float g_temperature = 0.0f;
float g_humidity    = 0.0f;


// ---------------------------------------------------------------------------
// WiFi
// ---------------------------------------------------------------------------
void connectToWifi() {
  log_i("Connecting to WiFi: %s", ssid);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  log_i("IP: %s", WiFi.localIP().toString().c_str());
}

void checkWiFi() {
  static unsigned long lastCheck = 0;
  unsigned long now = millis();
  if (now - lastCheck < WIFI_CHECK_INTERVAL_MS) return;
  lastCheck = now;

  if (WiFi.status() == WL_CONNECTED) return;

  log_w("WiFi lost — reconnecting");
  WiFi.disconnect();
  WiFi.begin(ssid, pass);
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) delay(500);

  if (WiFi.status() == WL_CONNECTED) {
    log_i("WiFi restored, IP: %s", WiFi.localIP().toString().c_str());
    server.begin();
  } else {
    log_w("WiFi reconnect failed, will retry");
  }
}


// ---------------------------------------------------------------------------
// MQTT / Home Assistant
// ---------------------------------------------------------------------------
void publishDiscovery() {
  char payload[512];

  snprintf(payload, sizeof(payload),
    "{\"name\":\"Temperature\","
    "\"state_topic\":\"%s\","
    "\"unit_of_measurement\":\"°F\","
    "\"device_class\":\"temperature\","
    "\"unique_id\":\"" DEVICE_ID "_temperature\","
    "\"device\":{\"identifiers\":[\"" HOSTNAME "\"],\"name\":\"" HOSTNAME "\","
    "\"model\":\"XIAO ESP32-C6 + SHT30\",\"manufacturer\":\"DIY\"}}",
    STATE_TEMP);
  mqtt.publish(DISCO_TEMP, payload, true);

  snprintf(payload, sizeof(payload),
    "{\"name\":\"Humidity\","
    "\"state_topic\":\"%s\","
    "\"unit_of_measurement\":\"%%\","
    "\"device_class\":\"humidity\","
    "\"unique_id\":\"" DEVICE_ID "_humidity\","
    "\"device\":{\"identifiers\":[\"" HOSTNAME "\"],\"name\":\"" HOSTNAME "\","
    "\"model\":\"XIAO ESP32-C6 + SHT30\",\"manufacturer\":\"DIY\"}}",
    STATE_HUMID);
  mqtt.publish(DISCO_HUMID, payload, true);

  log_i("HA discovery published");
}

void reconnectMQTT() {
  static unsigned long lastAttempt = 0;
  if (mqtt.connected()) return;
  if (WiFi.status() != WL_CONNECTED) return;

  unsigned long now = millis();
  if (now - lastAttempt < MQTT_RETRY_INTERVAL_MS) return;
  lastAttempt = now;

  log_i("Connecting to MQTT: %s", mqttServer);

  if (mqtt.connect(HOSTNAME, mqttUser, mqttPass)) {
    log_i("MQTT connected");
    publishDiscovery();
  } else {
    log_e("MQTT connect failed, rc=%d", mqtt.state());
  }
}

void publishSensorData() {
  if (!mqtt.connected()) return;

  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f", g_temperature * 9.0f / 5.0f + 32.0f);
  mqtt.publish(STATE_TEMP, buf);

  snprintf(buf, sizeof(buf), "%.1f", g_humidity);
  mqtt.publish(STATE_HUMID, buf);
}


// ---------------------------------------------------------------------------
// Web server — local status page
// ---------------------------------------------------------------------------
void setupWebServer() {
  server.begin();

  server.on("/", HTTP_GET, [](PsychicRequest* request, PsychicResponse* response) {
    char html[256];
    snprintf(html, sizeof(html),
      "<html><body><h2>Air Sensor</h2>"
      "<p>Temperature: %.1f F</p>"
      "<p>Humidity: %.1f %%</p>"
      "</body></html>",
      g_temperature * 9.0f / 5.0f + 32.0f, g_humidity);
    return response->send(200, "text/html", html);
  });

  server.on("/status", HTTP_GET, [](PsychicRequest* request, PsychicResponse* response) {
    char json[64];
    snprintf(json, sizeof(json),
      "{\"temperature\":%.1f,\"humidity\":%.1f}",
      g_temperature * 9.0f / 5.0f + 32.0f, g_humidity);
    return response->send(200, "application/json", json);
  });

  log_i("Web server started");
}


// ---------------------------------------------------------------------------
// E-paper display
// ---------------------------------------------------------------------------
void updateDisplay() {
  static float lastTemp  = NAN;
  static float lastHumid = NAN;

  if (!isnan(lastTemp) && !isnan(lastHumid) &&
      fabsf(g_temperature - lastTemp)  < 0.05f &&
      fabsf(g_humidity    - lastHumid) < 0.05f) return;

  lastTemp  = g_temperature;
  lastHumid = g_humidity;

  float tempF = g_temperature * 9.0f / 5.0f + 32.0f;
  char buf[16];

  epd.setRotation(1);  // landscape: 296 wide x 128 tall
  epd.setFullWindow();
  epd.firstPage();
  do {
    epd.fillScreen(GxEPD_BLACK);
    epd.setTextColor(GxEPD_WHITE);

    epd.setFont(&JosefinSans_Regular9pt7b);
    epd.setCursor(8, 18);
    epd.print("Temperature");

    epd.setFont(&JosefinSans_Bold18pt7b);
    snprintf(buf, sizeof(buf), "%.1f F", tempF);
    epd.setCursor(8, 70);
    epd.print(buf);

    epd.drawLine(148, 4, 148, 124, GxEPD_WHITE);

    epd.setFont(&JosefinSans_Regular9pt7b);
    epd.setCursor(158, 18);
    epd.print("Humidity");

    epd.setFont(&JosefinSans_Bold18pt7b);
    snprintf(buf, sizeof(buf), "%.1f %%", g_humidity);
    epd.setCursor(158, 70);
    epd.print(buf);

  } while (epd.nextPage());
  epd.hibernate();
}


// ---------------------------------------------------------------------------
// Setup / Loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);  // give USB CDC time to connect before any output

  // E-paper not connected yet — skipping EPD init
  // pinMode / SPI.begin / epd.init go here when display is wired up

  if (!sht30.begin(0x44)) {
    log_e("SHT30 not found");
    while (1) delay(10);
  }
  log_i("SHT30 OK");

  for (int i = 0; i < 5; i++) {
    g_temperature = sht30.readTemperature();
    g_humidity    = sht30.readHumidity();
    if (!isnan(g_temperature) && !isnan(g_humidity) &&
        g_temperature >= -40.0f && g_temperature <= 125.0f &&
        g_humidity >= 0.0f && g_humidity <= 100.0f) { break; }
    delay(500);
  }

  connectToWifi();
  mqtt.setServer(mqttServer, MQTT_PORT);
  mqtt.setBufferSize(512);
  reconnectMQTT();
  setupWebServer();
}

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  checkWiFi();
  reconnectMQTT();
  mqtt.loop();

  if (now - lastUpdate < UPDATE_INTERVAL_MS) return;
  lastUpdate = now;

  g_temperature = sht30.readTemperature();
  g_humidity    = sht30.readHumidity();

  if (isnan(g_temperature) || isnan(g_humidity) ||
      g_temperature < -40.0f || g_temperature > 125.0f ||
      g_humidity < 0.0f || g_humidity > 100.0f) {
    log_e("SHT30 read out of range: temp=%.1f hum=%.1f", g_temperature, g_humidity);
    return;
  }

  log_i("Temp: %.1fC (%.1fF)  Humidity: %.1f%%",
    g_temperature, g_temperature * 9.0f / 5.0f + 32.0f, g_humidity);

  // updateDisplay();  // re-enable when EPD is wired up
  publishSensorData();
}