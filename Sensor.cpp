#include "Sensor.h"

bool Sensor::begin()
{
    Wire.begin();

    if (imu.begin() != 0)
        return false;

    imuFound = true;
    lastMicros = micros();
    return true;
}

bool Sensor::read()
{
    if (!imuFound)
        return false;

    unsigned long now = micros();
    deltaTime = (now - lastMicros) / 1000000.0f;
    lastMicros = now;

    uint8_t d[12];
    imu.readRegisterRegion(d, 0x22, 12);

    gx = imu.calcGyro((int16_t)((d[1] << 8) | d[0]));
    gy = imu.calcGyro((int16_t)((d[3] << 8) | d[2]));
    gz = imu.calcGyro((int16_t)((d[5] << 8) | d[4]));

    ax = imu.calcAccel((int16_t)((d[7] << 8) | d[6]));
    ay = imu.calcAccel((int16_t)((d[9] << 8) | d[8]));
    az = imu.calcAccel((int16_t)((d[11] << 8) | d[10]));

    return true;
}