#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "LSM6DS3.h"

class Sensor {
public:
    float gx, gy, gz;
    float ax, ay, az;
    float deltaTime;

    bool begin();
    bool read();

private:
    LSM6DS3 imu = LSM6DS3(I2C_MODE, 0x6A);
    unsigned long lastMicros = 0;
    bool imuFound = false;
};