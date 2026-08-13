// ============================================================
// SERIAL KOMUTLARI
// ============================================================
//
// UZAY MODLARI:
//
// WORLDSPACE
// LOCALSPACE
// WANDSPACE
// PLAYERSPACE
//
// BAĞLANTI:
//
// WIRED
// BLE
//
// DİĞER:
//
// mcal  -> 1 saniyelik manuel kalibrasyon
// r     -> kartı yeniden başlat
//
// Başlangıç:
//   Mode       = WORLDSPACE
//   Connection = WIRED
//
// ============================================================


#include "Adafruit_TinyUSB.h"
#include <Wire.h>
#include "LSM6DS3.h"
#include "GamepadMotion.hpp"

#include <bluefruit.h>


// ============================================================
// USB HID
// ============================================================

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_MOUSE()
};

Adafruit_USBD_HID usb_hid(
    desc_hid_report,
    sizeof(desc_hid_report),
    HID_ITF_PROTOCOL_MOUSE,
    1,
    false
);


// ============================================================
// BLE HID
// ============================================================

BLEHidAdafruit blehid;


// ============================================================
// SENSÖR
// ============================================================

LSM6DS3 imu(I2C_MODE, 0x6A);

GamepadMotion motion;

bool imuFound = false;


// ============================================================
// ÇALIŞMA AYARLARI
// ============================================================

float sensitivity = 0.05f;

float accumX = 0.0f;
float accumY = 0.0f;

unsigned long lastMicros = 0;


// ============================================================
// SERIAL'DEN SEÇİLEN MOD
// ============================================================

enum MotionSpace
{
    SPACE_WORLD,
    SPACE_LOCAL,
    SPACE_WAND,
    SPACE_PLAYER
};

enum ConnectionMode
{
    CONNECTION_WIRED,
    CONNECTION_BLE
};


// Başlangıç ayarları
MotionSpace currentSpace = SPACE_WORLD;
ConnectionMode currentConnection = CONNECTION_WIRED;


// ============================================================
// MANUEL KALİBRASYON
// ============================================================

bool manualCalibrationActive = false;
unsigned long manualCalibrationStart = 0;


// ============================================================
// SERIAL KOMUT BUFFER
// ============================================================

char commandBuffer[20];
uint8_t commandIndex = 0;


// ============================================================
// BLE BAŞLAT
// ============================================================

void startBLE()
{
    Bluefruit.begin();

    Bluefruit.setTxPower(4);
    Bluefruit.setName("AirMouseBLE");

    blehid.begin();

    Bluefruit.Periph.setConnectCallback(
        [](uint16_t conn_handle)
        {
            BLEConnection* connection =
                Bluefruit.Connection(conn_handle);

            if (connection)
            {
                connection->requestConnectionParameter(
                    6,
                    0,
                    400
                );
            }
        }
    );

    Bluefruit.Periph.setDisconnectCallback(
        [](uint16_t conn_handle, uint8_t reason)
        {
            (void)conn_handle;
            (void)reason;
        }
    );

    Bluefruit.Advertising.stop();

    Bluefruit.Advertising.addFlags(
        BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE
    );

    Bluefruit.Advertising.addTxPower();

    Bluefruit.Advertising.addAppearance(
        BLE_APPEARANCE_HID_MOUSE
    );

    Bluefruit.Advertising.addService(blehid);

    Bluefruit.Advertising.addName();

    Bluefruit.Advertising.restartOnDisconnect(true);

    Bluefruit.Advertising.setInterval(
        32,
        244
    );

    Bluefruit.Advertising.setFastTimeout(30);

    Bluefruit.Advertising.start(0);
}


// ============================================================
// KOMUT İŞLEME
// ============================================================

void processCommand()
{
    if (commandIndex == 0)
        return;

    commandBuffer[commandIndex] = '\0';

    if (strcmp(commandBuffer, "WORLDSPACE") == 0)
    {
        currentSpace = SPACE_WORLD;
        Serial.println("Mode: WORLDSPACE");
    }
    else if (strcmp(commandBuffer, "LOCALSPACE") == 0)
    {
        currentSpace = SPACE_LOCAL;
        Serial.println("Mode: LOCALSPACE");
    }
    else if (strcmp(commandBuffer, "WANDSPACE") == 0)
    {
        currentSpace = SPACE_WAND;
        Serial.println("Mode: WANDSPACE");
    }
    else if (strcmp(commandBuffer, "PLAYERSPACE") == 0)
    {
        currentSpace = SPACE_PLAYER;
        Serial.println("Mode: PLAYERSPACE");
    }
    else if (strcmp(commandBuffer, "WIRED") == 0)
    {
        currentConnection = CONNECTION_WIRED;
        Serial.println("Connection: WIRED");
    }
    else if (strcmp(commandBuffer, "BLE") == 0)
    {
        currentConnection = CONNECTION_BLE;
        Serial.println("Connection: BLE");
    }
    else if (strcmp(commandBuffer, "mcal") == 0)
    {
        if (!manualCalibrationActive)
        {
            motion.ResetContinuousCalibration();
            motion.StartContinuousCalibration();
            manualCalibrationActive = true;
            manualCalibrationStart = millis();
            Serial.println("Manuel kalibrasyon basladi. 1 saniye sabit tut.");
        }
    }
    else if (strcmp(commandBuffer, "r") == 0 || strcmp(commandBuffer, "R") == 0)
    {
        Serial.println("Yeniden baslatiliyor...");
        delay(10);
        NVIC_SystemReset();
    }
    else
    {
        Serial.print("Bilinmeyen komut: ");
        Serial.println(commandBuffer);
    }

    commandIndex = 0;
}


void readSerialCommands()
{
    while (Serial.available() > 0)
    {
        char incomingChar = Serial.read();
        if (incomingChar == '\n' || incomingChar == '\r')
        {
            processCommand();
        }
        else
        {
            if (commandIndex < sizeof(commandBuffer) - 1)
            {
                commandBuffer[commandIndex] = incomingChar;
                commandIndex++;
            }
            else
            {
                commandIndex = 0;
            }
        }
    }
}


void setup()
{
    Serial.begin(115200);

    usb_hid.begin();
    startBLE();

#if defined(NRF52840_XXAA) && defined(PIN_LSM6DS3TR_C_POWER)
    pinMode(PIN_LSM6DS3TR_C_POWER, OUTPUT);
    digitalWrite(PIN_LSM6DS3TR_C_POWER, HIGH);
    delay(30);
#endif

    delay(100);

    if (imu.begin() == 0)
    {
        imuFound = true;
    }
    else
    {
        imuFound = false;
        while (1) delay(1000);
    }

    motion.SetCalibrationMode(
        GamepadMotionHelpers::CalibrationMode::Stillness |
        GamepadMotionHelpers::CalibrationMode::SensorFusion
    );

    for (int i = 0; i < 200; i++)
    {
        uint8_t d[12];
        imu.readRegisterRegion(d, 0x22, 12);

        float gx = imu.calcGyro((int16_t)((d[1] << 8) | d[0]));
        float gy = imu.calcGyro((int16_t)((d[3] << 8) | d[2]));
        float gz = imu.calcGyro((int16_t)((d[5] << 8) | d[4]));

        float ax = imu.calcAccel((int16_t)((d[7] << 8) | d[6]));
        float ay = imu.calcAccel((int16_t)((d[9] << 8) | d[8]));
        float az = imu.calcAccel((int16_t)((d[11] << 8) | d[10]));

        motion.ProcessMotion(-gy, gz, -gx, -ay, az, -ax, 0.01f);
        delay(10);
    }

    lastMicros = micros();

Serial.println();
Serial.println("================================");
Serial.println(" AirMouse baslatildi");
Serial.println("================================");
Serial.println("Mode: WORLDSPACE");
Serial.println("Connection: WIRED");
Serial.println();
Serial.println("Ilk manuel kalibrasyon basliyor...");
Serial.println("Kartı 1 saniye sabit tut.");
Serial.println("================================");

// Cihaz açılışında BİR KEZ otomatik manuel kalibrasyon
motion.ResetContinuousCalibration();
motion.StartContinuousCalibration();

manualCalibrationActive = true;
manualCalibrationStart = millis();

while (manualCalibrationActive)
{
    unsigned long now = micros();
    unsigned long elapsed = (unsigned long)(now - lastMicros);
    float deltaTime = elapsed / 1000000.0f;
    lastMicros = now;

    uint8_t d[12];
    imu.readRegisterRegion(d, 0x22, 12);

    float gx = imu.calcGyro((int16_t)((d[1] << 8) | d[0]));
    float gy = imu.calcGyro((int16_t)((d[3] << 8) | d[2]));
    float gz = imu.calcGyro((int16_t)((d[5] << 8) | d[4]));

    float ax = imu.calcAccel((int16_t)((d[7] << 8) | d[6]));
    float ay = imu.calcAccel((int16_t)((d[9] << 8) | d[8]));
    float az = imu.calcAccel((int16_t)((d[11] << 8) | d[10]));

    motion.ProcessMotion(
        -gy,
         gz,
        -gx,
        -ay,
         az,
        -ax,
        deltaTime
    );

    if ((unsigned long)(millis() - manualCalibrationStart) >= 1000)
    {
        motion.PauseContinuousCalibration();
        manualCalibrationActive = false;
    }

    delay(1);
}

Serial.println("Ilk manuel kalibrasyon tamamlandi.");
Serial.println();
Serial.println("Komutlar:");
Serial.println("WORLDSPACE");
Serial.println("LOCALSPACE");
Serial.println("WANDSPACE");
Serial.println("PLAYERSPACE");
Serial.println("WIRED");
Serial.println("BLE");
Serial.println("mcal");
Serial.println("r");
Serial.println("================================");
}


void loop()
{
    readSerialCommands();

    if (manualCalibrationActive)
    {
        if ((unsigned long)(millis() - manualCalibrationStart) >= 1000)
        {
            motion.PauseContinuousCalibration();
            manualCalibrationActive = false;
            Serial.println("Manuel kalibrasyon tamamlandi.");
        }
    }

    if (!imuFound) return;

    unsigned long now = micros();
    unsigned long elapsed = (unsigned long)(now - lastMicros);
    float deltaTime = elapsed / 1000000.0f;
    lastMicros = now;

    uint8_t d[12];
    imu.readRegisterRegion(d, 0x22, 12);

    float gx = imu.calcGyro((int16_t)((d[1] << 8) | d[0]));
    float gy = imu.calcGyro((int16_t)((d[3] << 8) | d[2]));
    float gz = imu.calcGyro((int16_t)((d[5] << 8) | d[4]));

    float ax = imu.calcAccel((int16_t)((d[7] << 8) | d[6]));
    float ay = imu.calcAccel((int16_t)((d[9] << 8) | d[8]));
    float az = imu.calcAccel((int16_t)((d[11] << 8) | d[10]));

    motion.ProcessMotion(-gy, gz, -gx, -ay, az, -ax, deltaTime);

    float mouseX = 0.0f;
    float mouseY = 0.0f;

    if (currentSpace == SPACE_WAND)
    {
        float gX = 0.0f, gY = 0.0f, gZ = 0.0f;
        motion.GetGravity(gX, gY, gZ);

        float cgX = 0.0f, cgY = 0.0f, cgZ = 0.0f;
        motion.GetCalibratedGyro(cgX, cgY, cgZ);

        float dotFG = -gZ;
        float fX = 0.0f - dotFG * gX;
        float fY = 0.0f - dotFG * gY;
        float fZ = -1.0f - dotFG * gZ;

        float lenF = sqrt(fX*fX + fY*fY + fZ*fZ);
        if (lenF > 0.001f) { fX /= lenF; fY /= lenF; fZ /= lenF; }
        else { fX = 0.0f; fY = 1.0f; fZ = 0.0f; }

        float rX = gY * fZ - gZ * fY;
        float rY = gZ * fX - gX * fZ;
        float rZ = gX * fY - gY * fX;

        float wPitch = cgX * rX + cgY * rY + cgZ * rZ;
        float wYaw   = cgX * (-gX) + cgY * (-gY) + cgZ * (-gZ);

        mouseX = -wYaw;
        mouseY = -wPitch;
    }
    else if (currentSpace == SPACE_WORLD)
    {
        float wsX = 0.0f, wsY = 0.0f;
        motion.GetWorldSpaceGyro(wsX, wsY);
        mouseX = -wsY;
        mouseY = -wsX;
    }
    else if (currentSpace == SPACE_PLAYER)
    {
        float psX = 0.0f, psY = 0.0f;
        motion.GetPlayerSpaceGyro(psX, psY);
        mouseX = -psY;
        mouseY = -psX;
    }
    else if (currentSpace == SPACE_LOCAL)
    {
        float cgX = 0.0f, cgY = 0.0f, cgZ = 0.0f;
        motion.GetCalibratedGyro(cgX, cgY, cgZ);
        mouseX = -cgY;
        mouseY = -cgX;
    }

    if (fabs(mouseX) < 0.10f) mouseX = 0.0f;
    if (fabs(mouseY) < 0.10f) mouseY = 0.0f;

    accumX += mouseX * sensitivity;
    accumY += mouseY * sensitivity;

    int16_t moveX = (int16_t)accumX;
    int16_t moveY = (int16_t)accumY;

    accumX -= moveX;
    accumY -= moveY;

    if (moveX == 0 && moveY == 0) return;

    moveX = constrain(moveX, -127, 127);
    moveY = constrain(moveY, -127, 127);

    if (currentConnection == CONNECTION_WIRED)
    {
        if (usb_hid.ready()) usb_hid.mouseMove(0, moveX, moveY);
    }
    else if (currentConnection == CONNECTION_BLE)
    {
        if (Bluefruit.connected()) blehid.mouseMove(moveX, moveY);
    }
}