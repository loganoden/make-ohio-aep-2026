// Sensor reading functions for temperature and light level

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void initSensors();
float getTemperature();
int getLightLevel();

#endif