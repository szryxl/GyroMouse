#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Sensor.h"
#include "MotionEngine.h"
#include "HIDManager.h"

class Commands {
public:
    void begin(Sensor &sensor, MotionEngine &motion, HIDManager &hid);
    void update();

private:
    Sensor *sensor;
    MotionEngine *motion;
    HIDManager *hid;

    char buffer[20];
    uint8_t index = 0;

    void process();
};