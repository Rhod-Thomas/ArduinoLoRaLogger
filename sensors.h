
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

#define MAX_SENSORS 4

void SensorStart(uint8_t sensorNo);
int16_t SensorRead(uint8_t sensorNo);

void SensorsInit();
bool SensorsService();

#endif
