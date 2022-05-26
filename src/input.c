#include "pit.h"
#include "input.h"
#include "arm2avr.h"

float gyro_offset = 512;
float angle = 180;

void InputInit(void)
{
    IoFromAvr.AdValue[TouchSensor] = 0x03FF;
    IoFromAvr.AdValue[GyroSensor] = 0x03FF;
    IoFromAvr.AdValue[MicSensor] = 0x03FF;
    IoFromAvr.AdValue[LightSensor] = 0x03FF;
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
    return IoFromAvr.AdValue[TouchSensor] < 200;
}

void InputGyroCalibrate()
{
    angle = 0;
}

float InputReadGyro()
{
    return (angle += 0.002 * 16 * ((int)IoFromAvr.AdValue[GyroSensor] - gyro_offset));
}
