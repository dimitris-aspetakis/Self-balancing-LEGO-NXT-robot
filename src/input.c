#include "input.h"
#include "arm2avr.h"
#include "pit.h"

float gyro_offset = 512;
float angle = 180;

void InputInit(void)
{
    IoFromAvr.AdValue[SensorPort00] = 0x03FF;
    IoFromAvr.AdValue[SensorPort01] = 0x03FF;
    IoFromAvr.AdValue[SensorPort02] = 0x03FF;
    IoFromAvr.AdValue[SensorPort03] = 0x03FF;
}

void InputExit(void)
{
    return;
}

void InputGetSensorValue(UWORD *value, UBYTE port)
{
    *value = IoFromAvr.AdValue[port];
}

UBYTE InputReadBatteryLevel()
{
    return (100 * (IoFromAvr.Battery & 0x03FF)) / 1023;
}

UBYTE InputTouchSensorActiveted()
{
    return IoFromAvr.AdValue[SensorPort00] < 200;
}

UBYTE InputTouchSensorActivetedDown()
{
    static int prev_value = 0;
    int value = IoFromAvr.AdValue[SensorPort00] < 200;
    if (prev_value == 1 && value == 1)
        return 0;
    return prev_value = value;
}

void InputGyroCalibrate()
{
    angle = 0;
}

void InputUpdateGyro()
{
    angle += 0.002 * 16 * ((int)IoFromAvr.AdValue[SensorPort01] - gyro_offset);
}

float InputReadGyro()
{
    return angle;
}
