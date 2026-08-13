#include "Config.h"
#include "Sensor.h"
#include "MotionEngine.h"
#include "HIDManager.h"
#include "Commands.h"

Sensor sensor;
MotionEngine motion;
HIDManager hid;
Commands commands;

void setup()
{
    Serial.begin(115200);

    sensor.begin();
    motion.begin();
    hid.begin();

    commands.begin(sensor, motion, hid);
}

void loop()
{
    commands.update();

    if (!sensor.read())
        return;

    motion.update(
        sensor.gx, sensor.gy, sensor.gz,
        sensor.ax, sensor.ay, sensor.az,
        sensor.deltaTime
    );

    float x, y;
    motion.getMouseMotion(x, y);
    hid.move(x, y);
}