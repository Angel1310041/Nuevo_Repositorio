#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <RCSwitch.h>
#include "esp_task_wdt.h"

#define LED_PIN 35
#define MQ6_PIN 19
#define prog 0
#define ROWS 3
#define COLS 3

struct SENSOR {
  int id;
  int zona;
  int tipo;
};

extern const char* TipoSensor[9][2];
extern boolean variableDetectada;
extern byte rowPins[ROWS];
extern byte colPins[COLS];


void setup();
void loop();

#endif // MAIN_H
