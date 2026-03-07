#include "sensors.h"
#include <math.h>

const int THERMISTOR_PIN = 34; // ADC1_CH6
const int LDR_PIN = 35; // ADC1_CH7 on ESP32 dev module

const float SERIES_RESISTOR = 10000.0;
const float NOMINAL_RESISTANCE = 10000.0;
const float NOMINAL_TEMPERATURE = 25.0;
const float B_COEFFICIENT = 3950.0;

void initSensors() {
  pinMode(THERMISTOR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
}

float getTemperature() {
  int adcValue = analogRead(THERMISTOR_PIN);
  if (adcValue <= 0 || adcValue >= 4095) return 0.0;
  
  float resistance = SERIES_RESISTOR / ((4095.0 / adcValue) - 1.0);
  
  float steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15; 
  
  return steinhart;
}

int getLightLevel() {
  return analogRead(LDR_PIN);
}