#include "pit.h"
#include "AT91SAM7S256.h"
#include "aic.h"


void PITEnable(void)
{
    *AT91C_PITC_PIMR = 0x01000BB8;
}

void PITDisable(void)
{
    *AT91C_PITC_PIMR = 0x00000000;
}

ULONG PITRead(void)
{
    return *AT91C_PITC_PIIR;
}

ULONG PITReadReset(void)
{
    return *AT91C_PITC_PIVR;
}

void PITInterruptEnable(ULONG period, void (*handler)(void))
{
    AICInterruptEnable(1, handler, 0);
    *AT91C_PITC_PIMR = *AT91C_PITC_PIMR & 0x03000000;
    *AT91C_PITC_PIMR = *AT91C_PITC_PIMR | (period * 0x00000BB8);
    *AT91C_PITC_PIMR = *AT91C_PITC_PIMR | 0x02000000;
}

void PITInterruptDisable(void)
{
    *AT91C_PITC_PIMR = *AT91C_PITC_PIMR & 0xFDFFFFFF;
    *AT91C_PITC_PIMR = *AT91C_PITC_PIMR & 0x03000000;
    AICInterruptDisable(1);
}

void PITAckInterrupt(void)
{
    PITReadReset();
}

UWORD PITTicks2ms(ULONG ticks)
{
    return ticks / 3000;
}

UWORD PITTicks2s(ULONG ticks)
{
    return ticks / 3000000;
}

void spindelayms(ULONG ms)
{
    ULONG PICNT;

    PITReadReset();
    while (1)
    {
        PICNT = PITRead() >> 20;
        if (PICNT >= ms)
            break;
    }
}
