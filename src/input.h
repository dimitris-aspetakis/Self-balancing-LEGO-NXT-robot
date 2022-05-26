#ifndef _INPUT_H
#define _INPUT_H
#include <stdconst.h>

extern float gyro_offset;
extern float angle;

enum InputSensor
{
    TouchSensor = 0,
    GyroSensor = 1,
    MicSensor = 2,
    LightSensor = 3,
};

void InputInit(void);
void InputExit(void);
void InputGetSensorValue(UWORD *value, UBYTE port);

UBYTE InputReadBatteryLevel();
UBYTE InputTouchSensorActiveted();
void InputGyroCalibrate();
float InputReadGyro();

#endif
