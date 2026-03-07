// The main loop: reads sensors, triggers LEDs/Fan, sends MQTT

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

// Pin Definitions