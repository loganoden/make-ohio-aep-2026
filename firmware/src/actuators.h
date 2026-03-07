#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

extern int currentFanSpeed;

void initActuators();
void updateWindSimulation();
float calculateHazardLevel(float temp);
void updateActuators(float hazardLevel);

#endif