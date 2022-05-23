#include "input.h"
#include "arm2avr.h"

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

UWORD InputReadGyro()
{
    angle += 0.002 * ((int)IoFromAvr.AdValue[GyroSensor] - 609);

    return (UWORD)angle;
}
