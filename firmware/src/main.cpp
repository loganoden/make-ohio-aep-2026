#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Import your modular files
#include "sensors.h"
#include "actuators.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastWifiRetry = 0;
unsigned long lastMqttRetry = 0;

const unsigned long WIFI_RETRY_MS = 10000;
const unsigned long MQTT_RETRY_MS = 5000;

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
}

void ensureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (now - lastWifiRetry >= WIFI_RETRY_MS) {
    lastWifiRetry = now;
    Serial.println("WiFi disconnected, retrying...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
  }
}

void reconnectMqttNonBlocking() {
  if (client.connected() || WiFi.status() != WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();
  if (now - lastMqttRetry >= MQTT_RETRY_MS) {
    lastMqttRetry = now;
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_GridSimulator")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize modular hardware
  initSensors();
  initActuators();
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  ensureWifiConnected();
  reconnectMqttNonBlocking();

  if (client.connected()) {
    client.loop();
  }

  updateWindSimulation();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { 
    lastMsg = now;

    float tempC = getTemperature();
    int lightLevel = getLightLevel();
    float hazard = calculateHazardLevel(tempC);

    updateActuators(hazard);

    JsonDocument doc;
    doc["temperature_c"] = tempC;
    doc["light_level"] = lightLevel;
    doc["hazard_level"] = hazard;
    doc["wind_pwm"] = currentFanSpeed;

    char buffer[256];
    serializeJson(doc, buffer);

    Serial.println(buffer);
    if (client.connected()) {
      bool ok = client.publish("grid/telemetry", buffer);
      if (!ok) {
        Serial.println("MQTT publish failed");
      }
    }
  }
}