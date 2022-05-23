#include "AT91SAM7S256.h"
#include "aic.h"
#include "arm2avr.h"
#include "button.h"
#include "display.h"
#include "hwinit.h"
#include "i2c.h"
#include "input.h"
#include "led.h"
#include "output.h"
#include "pit.h"
#include "sound.h"
#include <assert.h>
#include <stdlib.h>

int main(void)
{
    LedSwitchOff(LightSensor);
    HardwareInit(); // need this to init PIOA clock
    DisplayInit();
    DisplayErase();
    DisplayUpdateSync();
    PITEnable();
    AICInit();
    SoundInit();
    I2CInit();

    InputInit();
    ButtonInit();
    OutputInit();


    while (1)
    {
        DisplayErase();
        UWORD sensor_value;

        DisplayString(0, 0, (UBYTE *)ButtonToString[ButtonRead()]);


        InputGetSensorValue(&sensor_value, TouchSensor);
        DisplayString(0, 8, (UBYTE *)"Touch: ");
        InputTouchSensorActiveted() ? DisplayString(12 * 6, 8, (UBYTE *)"On")
                                    : DisplayString(12 * 6, 8, (UBYTE *)"Off");

        DisplayString(0, 16, (UBYTE *)"Gyro: ");
        DisplayNum(12 * 6, 16, InputReadGyro());

        InputGetSensorValue(&sensor_value, MicSensor);
        DisplayString(0, 24, (UBYTE *)"Mic: ");
        DisplayNum(12 * 6, 24, sensor_value);

        InputGetSensorValue(&sensor_value, LightSensor);
        DisplayString(0, 32, (UBYTE *)"Light: ");
        DisplayNum(12 * 6, 32, sensor_value);


        DisplayString(0, 40, (UBYTE *)"Motor Left: ");
        DisplayNum(12 * 6, 40, IoToAvr.PwmValue[0]);

        DisplayString(0, 48, (UBYTE *)"Motor Right: ");
        DisplayNum(12 * 6, 48, IoToAvr.PwmValue[2]);


        DisplayString(0, 56, (UBYTE *)"Battery(%): ");
        DisplayNum(12 * 6, 56, InputReadBatteryLevel());


        switch (ButtonRead())
        {
        case BUTTON_ENTER:
            I2CCtrl(REPROGRAM);
            break;
        case BUTTON_EXIT:
            I2CCtrl(POWERDOWN);
            break;
        case BUTTON_LEFT:
            LedSwitchOff(2);
            break;
        case BUTTON_RIGHT:
            InputGyroCalibrate();
            break;
        case BUTTON_NONE:
            break;
        }


        DisplayUpdateSync();
        I2CTransfer();
    }

    ButtonExit();
    InputExit();
    I2CExit();
    PITInterruptDisable();
    PITDisable();
    SoundExit();
    DisplayExit();

    return 0;
}