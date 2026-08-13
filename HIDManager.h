#pragma once
#include <Arduino.h>
#include "Config.h"

class HIDManager {
public:
    void begin();
    void move(float x, float y);
    void setConnection(ConnectionMode mode);

private:
    ConnectionMode currentConnection = CONNECTION_WIRED;
    float accumX = 0.0f;
    float accumY = 0.0f;
};