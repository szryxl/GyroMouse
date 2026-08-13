#pragma once

#define AIR_MOUSE_NAME "AirMouseBLE"
#define DEFAULT_SENSITIVITY 0.05f
#define MOUSE_DEADZONE 0.10f
#define IMU_ADDRESS 0x6A

enum MotionSpace {
    SPACE_WORLD,
    SPACE_LOCAL,
    SPACE_WAND,
    SPACE_PLAYER
};

enum ConnectionMode {
    CONNECTION_WIRED,
    CONNECTION_BLE
};