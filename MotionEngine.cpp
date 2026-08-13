#include "MotionEngine.h"

void MotionEngine::begin()
{
    motion.SetCalibrationMode(
        GamepadMotionHelpers::CalibrationMode::Stillness |
        GamepadMotionHelpers::CalibrationMode::SensorFusion
    );
}

void MotionEngine::update(float gx, float gy, float gz, float ax, float ay, float az, float dt)
{
    motion.ProcessMotion(-gy, gz, -gx, -ay, az, -ax, dt);
}

void MotionEngine::setSpace(MotionSpace space)
{
    currentSpace = space;
}

bool MotionEngine::startCalibration()
{
    if (calibrationActive)
        return false;

    motion.ResetContinuousCalibration();
    motion.StartContinuousCalibration();

    calibrationActive = true;
    calibrationStart = millis();

    return true;
}

bool MotionEngine::updateCalibration()
{
    if (!calibrationActive)
        return false;

    if (millis() - calibrationStart < 1000)
        return false;

    motion.PauseContinuousCalibration();
    calibrationActive = false;

    return true;
}

void MotionEngine::getMouseMotion(float &x, float &y)
{
    x = 0.0f;
    y = 0.0f;

    switch (currentSpace)
    {
        case SPACE_WORLD:  worldSpace(x, y);  break;
        case SPACE_LOCAL:  localSpace(x, y);  break;
        case SPACE_WAND:   wandSpace(x, y);   break;
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
    float gX, gY, gZ;
    float cgX, cgY, cgZ;

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