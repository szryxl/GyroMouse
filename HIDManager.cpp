#include "HIDManager.h"
#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>

uint8_t const hidReport[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

Adafruit_USBD_HID usbHid(
    hidReport,
    sizeof(hidReport),
    HID_ITF_PROTOCOL_MOUSE,
    1,
    false
);

BLEHidAdafruit bleHid;

static bool bleStarted = false;

void HIDManager::begin()
{
    usbHid.begin();
}

void HIDManager::setConnection(ConnectionMode mode)
{
    currentConnection = mode;

    if (mode == CONNECTION_BLE && !bleStarted)
    {
        Bluefruit.begin();
        Bluefruit.setTxPower(4);
        Bluefruit.setName(AIR_MOUSE_NAME);

        bleHid.begin();

        Bluefruit.Advertising.addFlags(
            BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE
        );
        Bluefruit.Advertising.addTxPower();
        Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
        Bluefruit.Advertising.addService(bleHid);
        Bluefruit.Advertising.addName();
        Bluefruit.Advertising.restartOnDisconnect(true);
        Bluefruit.Advertising.setInterval(32, 244);
        Bluefruit.Advertising.setFastTimeout(30);
        Bluefruit.Advertising.start(0);

        bleStarted = true;
    }
}

void HIDManager::move(float x, float y)
{
    if (fabs(x) < MOUSE_DEADZONE) x = 0.0f;
    if (fabs(y) < MOUSE_DEADZONE) y = 0.0f;

    accumX += x * DEFAULT_SENSITIVITY;
    accumY += y * DEFAULT_SENSITIVITY;

    int16_t moveX = (int16_t)accumX;
    int16_t moveY = (int16_t)accumY;

    accumX -= moveX;
    accumY -= moveY;

    if (moveX == 0 && moveY == 0)
        return;

    moveX = constrain(moveX, -127, 127);
    moveY = constrain(moveY, -127, 127);

    if (currentConnection == CONNECTION_WIRED)
    {
        if (usbHid.ready())
            usbHid.mouseMove(0, moveX, moveY);
    }
    else if (bleStarted && Bluefruit.connected())
    {
        bleHid.mouseMove(moveX, moveY);
    }
}