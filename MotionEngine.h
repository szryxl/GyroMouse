#pragma once
#include <Arduino.h>
#include "GamepadMotion.hpp"
#include "Config.h"

class Sensor;

class MotionEngine
{
public:
    void begin();
    void update(float gx, float gy, float gz, float ax, float ay, float az, float dt);
    void getMouseMotion(float &x, float &y);
    void setSpace(MotionSpace space);
    void calibrate(Sensor &sensor, bool startup = false);

private:
    GamepadMotion motion;
    MotionSpace currentSpace = SPACE_WORLD;

    void worldSpace(float &x, float &y);
    void localSpace(float &x, float &y);
    void wandSpace(float &x, float &y);
    void playerSpace(float &x, float &y);
};