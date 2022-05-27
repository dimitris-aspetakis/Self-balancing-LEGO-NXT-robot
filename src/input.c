#include <math.h>

#include "arm2avr.h"
#include "input.h"
#include "pit.h"

float gyro_offset = 609;
float angle = 180;
// float buffer[4];

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

void InputGyroReset()
{
    angle = 0;
}

void InputUpdateGyro()
{
    angle += 0.002 * 16 * ((int)IoFromAvr.AdValue[SensorPort01] - gyro_offset);
    // float h, a, b, c, d;
    // buffer[3] = buffer[2];
    // buffer[2] = buffer[1];
    // buffer[1] = buffer[0];
    // buffer[0] = (int)IoFromAvr.AdValue[SensorPort01] - 609;
    // h = 0.002 * 16;
    // a = buffer[0];
    // b = buffer[1];
    // c = buffer[2];
    // d = buffer[3];
    // angle += h/6*(a + 2*b + 2*c + d);
}

float InputReadGyro()
{
    return angle;
}
