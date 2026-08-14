#include "MotionEngine.h"
#include "Sensor.h"

void MotionEngine::begin()
{
    motion.SetCalibrationMode(
        GamepadMotionHelpers::CalibrationMode::Stillness |
        GamepadMotionHelpers::CalibrationMode::SensorFusion
    );
}

void MotionEngine::calibrate(Sensor &sensor, bool startup)
{
    if (startup)
    {
        for (int i = 0; i < 200; i++)
        {
            if (sensor.read())
            {
                motion.ProcessMotion(
                    -sensor.gy,
                     sensor.gz,
                    -sensor.gx,
                    -sensor.ay,
                     sensor.az,
                    -sensor.ax,
                    0.01f
                );
            }

            delay(10);
        }
    }

    motion.ResetContinuousCalibration();
    motion.StartContinuousCalibration();

    unsigned long start = millis();

    while ((unsigned long)(millis() - start) < 1000)
    {
        if (sensor.read())
        {
            motion.ProcessMotion(
                -sensor.gy,
                 sensor.gz,
                -sensor.gx,
                -sensor.ay,
                 sensor.az,
                -sensor.ax,
                sensor.deltaTime
            );
        }

        delay(1);
    }

    motion.PauseContinuousCalibration();
}

void MotionEngine::update(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    motion.ProcessMotion(-gy, gz, -gx, -ay, az, -ax, dt);
}

void MotionEngine::setSpace(MotionSpace space)
{
    currentSpace = space;
}

void MotionEngine::getMouseMotion(float &x, float &y)
{
    x = 0.0f;
    y = 0.0f;

    switch (currentSpace)
    {
        case SPACE_WORLD: worldSpace(x, y); break;
        case SPACE_LOCAL: localSpace(x, y); break;
        case SPACE_WAND: wandSpace(x, y); break;
        case SPACE_PLAYER: playerSpace(x, y); break;
    }
}

void MotionEngine::worldSpace(float &x, float &y)
{
    float gx, gy;
    motion.GetWorldSpaceGyro(gx, gy);
    x = -gy;
    y = -gx;
}

void MotionEngine::localSpace(float &x, float &y)
{
    float gx, gy, gz;
    motion.GetCalibratedGyro(gx, gy, gz);
    x = -gy;
    y = -gx;
}

void MotionEngine::playerSpace(float &x, float &y)
{
    float gx, gy;
    motion.GetPlayerSpaceGyro(gx, gy);
    x = -gy;
    y = -gx;
}

void MotionEngine::wandSpace(float &x, float &y)
{
    float gX, gY, gZ, cgX, cgY, cgZ;

    motion.GetGravity(gX, gY, gZ);
    motion.GetCalibratedGyro(cgX, cgY, cgZ);

    float dotFG = -gZ;
    float fX = -dotFG * gX;
    float fY = -dotFG * gY;
    float fZ = -1.0f - dotFG * gZ;

    float lenF = sqrt(fX * fX + fY * fY + fZ * fZ);

    if (lenF > 0.001f)
    {
        fX /= lenF;
        fY /= lenF;
        fZ /= lenF;
    }
    else
    {
        fX = 0.0f;
        fY = 1.0f;
        fZ = 0.0f;
    }

    float rX = gY * fZ - gZ * fY;
    float rY = gZ * fX - gX * fZ;
    float rZ = gX * fY - gY * fX;

    float wPitch = cgX * rX + cgY * rY + cgZ * rZ;
    float wYaw = cgX * (-gX) + cgY * (-gY) + cgZ * (-gZ);

    x = -wYaw;
    y = -wPitch;
}