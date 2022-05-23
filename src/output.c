#include "output.h"
#include "arm2avr.h"


void OutputInit(void)
{
    IoToAvr.Power = 0x00;
    IoToAvr.PwmFreq = 0x08;
    for (unsigned i = 0; i < NOS_OF_AVR_OUTPUTS; ++i)
        IoToAvr.PwmValue[i] = 0x00;
    IoToAvr.OutputMode = 0x00;
    IoToAvr.InputPower = 0x00;
}

void OutputExit(void)
{
    return;
}

void OutputSetSpeed(UBYTE MotorNr, SBYTE Speed)
{
    IoToAvr.PwmValue[MotorNr] = Speed;
}
