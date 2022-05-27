#ifndef _INPUT_H
#define _INPUT_H
#include <stdconst.h>

extern float gyro_offset;
extern float angle;

enum InputSensor
{
    SensorPort00 = 0,
    SensorPort01 = 1,
    SensorPort02 = 2,
    SensorPort03 = 3,
};

void InputInit(void);
void InputExit(void);
void InputGetSensorValue(UWORD *value, UBYTE port);

UBYTE InputReadBatteryLevel();
UBYTE InputTouchSensorActiveted();
UBYTE InputTouchSensorActivetedDown();
void InputGyroReset();
void InputUpdateGyro();
float InputReadGyro();

#endif
