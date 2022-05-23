#include "i2c.h"
#include "AT91SAM7S256.h"
#include "aic.h"
#include "arm2avr.h"
#include "display.h"
#include "pit.h"
#include <assert.h>
#include <stdconst.h>
#include <string.h>

#define BYTES_TO_TX sizeof(IOTOAVR)
#define BYTES_TO_RX sizeof(IOFROMAVR)
#define TIMEOUT 2100
#define I2CClk 400000L
#define TIME400KHz (((OSC / 16L) / (I2CClk * 2)) + 1)
#define CLDIV (((OSC / I2CClk) / 2) - 3)
#define DEVICE_ADR 0x01
#define COPYRIGHTSTRING "Let's samba nxt arm in arm, (c)LEGO System A/S"
#define COPYRIGHTSTRINGLENGTH 46
const UBYTE CopyrightStr[] = {"\xCC" COPYRIGHTSTRING};
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define piir (*AT91C_PITC_PIIR & AT91C_PITC_CPIV)
#define pimr (*AT91C_PITC_PIMR & AT91C_PITC_CPIV)
#define DISABLEI2cIrqs *AT91C_TWI_IDR = 0x000001C7

#define WAITClk(t)                                                             \
    {                                                                          \
        ULONG pit = piir + (t);                                                \
        if (pit >= pimr)                                                       \
            pit -= pimr;                                                       \
        while (piir < pit)                                                     \
        {                                                                      \
            ;                                                                  \
        }                                                                      \
    }

// send/receive state machine variables
enum state_t
{
    Hello,
    Tx_Start,
    Tx_Next,
    Rx_Start,
    Rx_Next
};

static UBYTE volatile tmp_buf[sizeof(CopyrightStr) + 1];
static UBYTE volatile checksum;


/*******************************************************************************
 * Asynchronous I2C transaction functions                                      *
 ******************************************************************************/
static enum state_t volatile State = Hello;
static size_t volatile transaction_bytes;
static size_t volatile byte_counter;


__ramfunc void I2cHandler(void)
{
    if (State == Rx_Next)
    {
        if (byte_counter < transaction_bytes - 2)
        {
            tmp_buf[byte_counter] = *AT91C_TWI_RHR;
            checksum += tmp_buf[byte_counter++];
        }
        else if (byte_counter == transaction_bytes - 2)
        {
            tmp_buf[byte_counter] = *AT91C_TWI_RHR;
            *AT91C_TWI_CR = AT91C_TWI_STOP;
            checksum += tmp_buf[byte_counter++];
            checksum = ~checksum;
        }
        else if (byte_counter >= transaction_bytes - 1)
        {
            DISABLEI2cIrqs;
            AICInterruptDisable(AT91C_ID_TWI);
            if (*AT91C_TWI_RHR == checksum)
                memcpy(&IoFromAvr, (void *)tmp_buf, transaction_bytes - 1);
            State = Tx_Start;
        }
    }
    else if (State == Tx_Next)
    {
        if (byte_counter < transaction_bytes - 1)
        {
            *AT91C_TWI_THR = tmp_buf[byte_counter++];
        }
        else if (byte_counter == transaction_bytes - 1)
        {
            *AT91C_TWI_CR = AT91C_TWI_STOP;
            *AT91C_TWI_THR = tmp_buf[byte_counter++];
        }
        else if (byte_counter >= transaction_bytes)
        {
            DISABLEI2cIrqs;
            AICInterruptDisable(AT91C_ID_TWI);
            State = Rx_Start;
        }
    }

    return;
}

void DataTxInit(UBYTE *buf, UBYTE len)
{
    transaction_bytes = len + 1;
    byte_counter = 0;
    checksum = 0x00;

    for (size_t i = 0; i < len; ++i)
        checksum += buf[i];
    checksum = ~checksum;

    memcpy((void *)tmp_buf, buf, len);
    tmp_buf[len] = checksum;

    *AT91C_TWI_IER = AT91C_TWI_TXRDY;
    AICInterruptEnable(AT91C_ID_TWI, (void *)I2cHandler,
                       AT91C_AIC_PRIOR_HIGHEST);

    *AT91C_TWI_MMR = (DEVICE_ADR << 16);
    *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;

    while (!(*AT91C_TWI_SR & AT91C_TWI_TXRDY))
        ;
    *AT91C_TWI_THR = tmp_buf[0];
    byte_counter += 1;

    return;
}

void DataRxInit(void)
{
    transaction_bytes = BYTES_TO_RX + 1;
    byte_counter = 0;
    checksum = 0x00;

    *AT91C_TWI_IER = AT91C_TWI_RXRDY;
    AICInterruptEnable(AT91C_ID_TWI, (void *)I2cHandler,
                       AT91C_AIC_PRIOR_HIGHEST);

    *AT91C_TWI_MMR = (DEVICE_ADR << 16) | AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD;
    *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;

    return;
}

void I2CTransfer(void)
{
    while ((PITRead() >> 20) < 2)
        ;

    switch (State)
    {
    case Hello:
        DataTxInit((UBYTE *)CopyrightStr, sizeof(CopyrightStr));
        State = Tx_Next;
        break;
    case Tx_Start:
        DataTxInit((UBYTE *)&IoToAvr, BYTES_TO_TX);
        State = Tx_Next;
        break;
    case Rx_Start:
        DataRxInit();
        State = Rx_Next;
        break;
    case Tx_Next:
    case Rx_Next:
        break;
    }

    PITReadReset();
    return;
}
/*******************************************************************************
 * End of asynchronous I2C transaction functions                               *
 ******************************************************************************/


/*******************************************************************************
 * Synchronous I2C transaction functions                                       *
 ******************************************************************************/
void DataTxSync(UBYTE *buf, UBYTE len)
{
    checksum = 0x00;

    for (size_t i = 0; i < len; ++i)
        checksum += buf[i];
    checksum = ~checksum;

    memcpy((void *)tmp_buf, buf, len);
    tmp_buf[len] = checksum;

    *AT91C_TWI_MMR = (DEVICE_ADR << 16) | AT91C_TWI_IADRSZ_NO;
    *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;

    for (size_t i = 0; i < len + 1; ++i)
    {
        while (!(*AT91C_TWI_SR & AT91C_TWI_TXRDY))
            ;
        if (i == len)
            *AT91C_TWI_CR = AT91C_TWI_STOP;
        *AT91C_TWI_THR = tmp_buf[i];
    }

    return;
}

void DataRxSync(void)
{
    checksum = 0x00;

    *AT91C_TWI_MMR = (DEVICE_ADR << 16) | AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD;
    *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;

    for (size_t i = 0; i < BYTES_TO_RX + 1; ++i)
    {
        while (!(*AT91C_TWI_SR & AT91C_TWI_RXRDY))
            ;
        tmp_buf[i] = *AT91C_TWI_RHR;
        if (i == BYTES_TO_RX - 1)
            *AT91C_TWI_CR = AT91C_TWI_STOP;
        checksum += tmp_buf[i];
    }

    if (checksum == 0xFF)
        memcpy(&IoFromAvr, (void *)tmp_buf, BYTES_TO_RX);

    return;
}

void I2CTransferInitSync()
{
    DataTxSync((UBYTE *)CopyrightStr, sizeof(CopyrightStr));
    spindelayms(2);
}

void I2CTransferSync()
{
    DataRxSync();
    spindelayms(2);
    DataTxSync((UBYTE *)&IoToAvr, BYTES_TO_TX);
    spindelayms(2);
}
/*******************************************************************************
 * End of synchronous I2C transaction functions                                *
 ******************************************************************************/


void I2CCtrl(enum power_t p)
{
    IoToAvr.Power = p >> 8;
    IoToAvr.PwmFreq = p;

    return;
}

void I2CInit(void)
{
    //
    // disable I2C on PIO
    // this is called also during an error, so interrupts etc may be enabled
    //
    *AT91C_AIC_IDCR = (1L << AT91C_ID_TWI);               /* disable AIC irq  */
    DISABLEI2cIrqs;                                       /* disable TWI irq  */
    *AT91C_PMC_PCER = (1L << AT91C_ID_TWI);               /* enable TWI Clock */
    *AT91C_PIOA_OER = AT91C_PA4_TWCK;                     /* SCL is output    */
    *AT91C_PIOA_ODR = AT91C_PA3_TWD;                      /* SDA is input     */
    *AT91C_PIOA_MDER = (AT91C_PA4_TWCK | AT91C_PA3_TWD);  /* open drain       */
    *AT91C_PIOA_PPUDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* no pull up       */
    //
    // synch I2C clocks using PIO
    // generate a 400KHz pulse on SCK and wait until both SCK and SDA are high,
    // which means the slave ticks with this clock
    //
    *AT91C_PIOA_PER = (AT91C_PA4_TWCK |
                       AT91C_PA3_TWD); /* enable PIO control for these pins */
    while (((*AT91C_PIOA_PDSR & AT91C_PA3_TWD) == 0) ||
           ((*AT91C_PIOA_PDSR & AT91C_PA4_TWCK) == 0))
    {
        *AT91C_PIOA_CODR = AT91C_PA4_TWCK; /* drive SCL Low  */
        WAITClk(TIME400KHz);
        *AT91C_PIOA_SODR = AT91C_PA4_TWCK; /* drive SCL High */
        WAITClk(TIME400KHz);
    }
    //
    // init I2C on PIO
    //
    *AT91C_TWI_CR = AT91C_TWI_SWRST; /* this has to happen before the rest */
    *AT91C_PIOA_PDR = (AT91C_PA4_TWCK |
                       AT91C_PA3_TWD); /* disable PIO control for these pins */
    *AT91C_PIOA_ASR =
        (AT91C_PA4_TWCK | AT91C_PA3_TWD);     /* select peripheral A = TWI */
    *AT91C_TWI_CWGR = (CLDIV | (CLDIV << 8)); /* 400KHz clock    */
    *AT91C_AIC_ICCR = (1L << AT91C_ID_TWI);   /* clear AIC irq   */
    AT91C_AIC_SVR[AT91C_ID_TWI] = (unsigned int)I2cHandler;
    AT91C_AIC_SMR[AT91C_ID_TWI] =
        ((AT91C_AIC_PRIOR_HIGHEST) | (AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED));
    *AT91C_AIC_IECR = (1L << AT91C_ID_TWI); /* Enables AIC irq */

    IoToAvr.Power = 0;

    return;
}

void I2CExit(void)
{
    DISABLEI2cIrqs;
    *AT91C_AIC_IDCR = (1L << AT91C_ID_TWI);               /* Disable AIC irq  */
    *AT91C_AIC_ICCR = (1L << AT91C_ID_TWI);               /* Clear AIC irq    */
    *AT91C_PMC_PCDR = (1L << AT91C_ID_TWI);               /* Disable clock    */
    *AT91C_PIOA_MDER = (AT91C_PA4_TWCK | AT91C_PA3_TWD);  /* Open drain       */
    *AT91C_PIOA_PPUDR = (AT91C_PA4_TWCK | AT91C_PA3_TWD); /* no pull up       */
    *AT91C_PIOA_PER = (AT91C_PA4_TWCK | AT91C_PA3_TWD);   /* Disable peripheal*/
}
