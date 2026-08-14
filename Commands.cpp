#include "Commands.h"

void Commands::begin(Sensor &s, MotionEngine &m, HIDManager &h)
{
    sensor = &s;
    motion = &m;
    hid = &h;
}

void Commands::update()
{
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
            process();
        else if (index < sizeof(buffer) - 1)
            buffer[index++] = c;
        else
            index = 0;
    }
}

void Commands::process()
{
    if (!index)
        return;

    buffer[index] = '\0';

    if (!strcmp(buffer, "WORLDSPACE"))
    {
        motion->setSpace(SPACE_WORLD);
        Serial.println("Mode: WORLDSPACE");
    }
    else if (!strcmp(buffer, "LOCALSPACE"))
    {
        motion->setSpace(SPACE_LOCAL);
        Serial.println("Mode: LOCALSPACE");
    }
    else if (!strcmp(buffer, "WANDSPACE"))
    {
        motion->setSpace(SPACE_WAND);
        Serial.println("Mode: WANDSPACE");
    }
    else if (!strcmp(buffer, "PLAYERSPACE"))
    {
        motion->setSpace(SPACE_PLAYER);
        Serial.println("Mode: PLAYERSPACE");
    }
    else if (!strcmp(buffer, "WIRED"))
    {
        hid->setConnection(CONNECTION_WIRED);
        Serial.println("Connection: WIRED");
    }
    else if (!strcmp(buffer, "BLE"))
    {
        hid->setConnection(CONNECTION_BLE);
        Serial.println("Connection: BLE");
    }
    else if (!strcmp(buffer, "mcal"))
    {
        Serial.println("Calibration started. Keep still.");
        motion->calibrate(*sensor);
        Serial.println("Calibration completed.");
    }
    else if (!strcmp(buffer, "r") || !strcmp(buffer, "R"))
    {
        NVIC_SystemReset();
    }
    else
    {
        Serial.print("Unknown: ");
        Serial.println(buffer);
    }

    index = 0;
}