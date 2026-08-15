#pragma once

#include <Arduino.h>
#include "Config.h"

class HIDManager
{
public:
    void begin();
    void move(float x, float y);
    void setConnection(ConnectionMode mode);
    void setSensitivity(float value);

private:
    ConnectionMode currentConnection = CONNECTION_WIRED;

    float sensitivity = DEFAULT_SENSITIVITY;

    float accumX = 0.0f;
    float accumY = 0.0f;

    bool bleInitialized = false;
};