// Logic for driving the LED GPIOs and Fan PWM

#include "actuators.h"

const int GREEN_LED = 27;
const int YELLOW_LED = 26;
const int RED_LED = 25;
const int BUZZER_PIN = 32;     
const int FAN_PWM_PIN = 33;    

unsigned long lastWindChange = 0;
int currentFanSpeed = 0;

void initActuators() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PWM_PIN, OUTPUT);
}

void updateWindSimulation() {
  if (millis() - lastWindChange > 3000) {
    currentFanSpeed = random(100, 256); 
    analogWrite(FAN_PWM_PIN, currentFanSpeed);
    lastWindChange = millis();
  }
}

float calculateHazardLevel(float temp) {
  float hazard = (temp - 30.0) / (80.0 - 30.0);
  if (hazard < 0.0) hazard = 0.0;
  if (hazard > 1.0) hazard = 1.0;
  return hazard;
}

void updateActuators(float hazardLevel) {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  if (hazardLevel < 0.4) {
    digitalWrite(GREEN_LED, HIGH);
    analogWrite(BUZZER_PIN, 0); 
  } 
  else if (hazardLevel >= 0.4 && hazardLevel < 0.8) {
    digitalWrite(YELLOW_LED, HIGH);
    analogWrite(BUZZER_PIN, int(hazardLevel * 128)); 
  } 
  else {
    digitalWrite(RED_LED, HIGH);
    analogWrite(BUZZER_PIN, int(hazardLevel * 255)); 
  }
}