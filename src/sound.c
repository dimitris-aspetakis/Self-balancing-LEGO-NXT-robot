#include "sound.h"
#include "AT91SAM7S256.h"
#include "aic.h"
#include <stdconst.h>


ULONG *sound_pattern;
UBYTE sound_length;
UWORD sound_duration;


void SoundEnable()
{
    *AT91C_PIOA_PER = AT91C_PA17_TD;
}

void SoundDisable()
{
    *AT91C_PIOA_PDR = AT91C_PA17_TD;
}

void SoundIntEnable()
{
    *AT91C_SSC_IER = AT91C_SSC_TXRDY;
}

void SoundIntDisable()
{
    *AT91C_SSC_IDR = 0x00000FFF;
}

void SoundInit(void)
{
    SoundIntDisable();
    SoundDisable();
    *AT91C_PMC_PCER = (1L << AT91C_ID_SSC); /* Enable MCK clock */
    *AT91C_PIOA_ODR = AT91C_PA17_TD;
    *AT91C_PIOA_OWDR = AT91C_PA17_TD;
    *AT91C_PIOA_MDDR = AT91C_PA17_TD;
    *AT91C_PIOA_PPUDR = AT91C_PA17_TD;
    *AT91C_PIOA_IFDR = AT91C_PA17_TD;
    *AT91C_PIOA_CODR = AT91C_PA17_TD;
    *AT91C_PIOA_IDR = AT91C_PA17_TD;
    *AT91C_SSC_CR = AT91C_SSC_SWRST;
    *AT91C_SSC_TCMR =
        AT91C_SSC_CKS_DIV + AT91C_SSC_CKO_CONTINOUS + AT91C_SSC_START_CONTINOUS;
    *AT91C_SSC_TFMR =
        (-1) + (((sizeof(ULONG) * 8) & 0xF) << 8) + AT91C_SSC_MSBF;
    *AT91C_SSC_CR = AT91C_SSC_TXEN; /* TX enable */
}

void SoundSync(ULONG *pattern, UBYTE length, UBYTE rate, UWORD duration)
{
    /* sound frequency will be (OSC/CMR*2)
       with CMR divisor values of 12 bits we can get a frequency range of 6KHz
       to 24MHz use rate values 0..255 to represent the range of 6KHz..24KHz, so
       div values 1K..4K for rate=0 => div=4K => freq ~= 6KHz and for rate=255
       => div=1K => freq ~= 24KHz *AT91C_SSC_CMR   = ((OSC /
       (2*sizeof(ULONG)*8)) / rate) + 1;
     */
    UBYTE i = 0;
    SoundEnable();
    *AT91C_SSC_CMR = (4095 - 12 * rate);
    while (duration-- > 0)
    {
        for (i = 0; i < length; i++)
        {
            while (!(*AT91C_SSC_SR & AT91C_SSC_TXRDY))
            { /* wait */
                ;
            };
            *AT91C_SSC_THR = pattern[i];
        }
    }
    SoundDisable();
    return;
}

void sound_handler()
{
    static UBYTE i = 1;

    if (i == sound_length)
    {
        i = 0;
        --sound_duration;
    }

    if (sound_duration)
    {
        *AT91C_SSC_THR = sound_pattern[i++];
    }
    else
    {
        i = 1;
        SoundDisable();
        SoundIntDisable();
    }
}

void SoundAsync(ULONG *pattern, UBYTE length, UBYTE rate, UWORD duration)
{
    /* sound frequency will be (OSC/CMR*2)
       with CMR divisor values of 12 bits we can get a frequency range of 6KHz
       to 24MHz use rate values 0..255 to represent the range of 6KHz..24KHz, so
       div values 1K..4K for rate=0 => div=4K => freq ~= 6KHz and for rate=255
       => div=1K => freq ~= 24KHz *AT91C_SSC_CMR   = ((OSC /
       (2*sizeof(ULONG)*8)) / rate) + 1;
     */
    SoundEnable();
    *AT91C_SSC_CMR = (4095 - 12 * rate);
    sound_pattern = pattern;
    sound_length = length;
    sound_duration = duration;
    *AT91C_SSC_THR = sound_pattern[0];
    AICInterruptEnable(8, sound_handler, 1);
    SoundIntEnable();
}

void SoundExit(void)
{
    *AT91C_SSC_CR = AT91C_SSC_TXDIS; /* TX disable */
}
