#include "button.h"
#include "arm2avr.h"

char *ButtonToString[] = {
    "BUTTON_NONE", "BUTTON_LEFT", "BUTTON_ENTER", "BUTTON_RIGHT", "BUTTON_EXIT",
};

void ButtonInit(void)
{
    IoFromAvr.Buttons = 0x0000;
}

void ButtonExit(void)
{
    return;
}

enum button_t ButtonRead(void)
{
    switch (IoFromAvr.Buttons)
    {
    case 127:
        return BUTTON_LEFT;
    case 406:
        return BUTTON_RIGHT;
    case 2047:
        return BUTTON_ENTER;
    case 1023:
        return BUTTON_EXIT;
    default:
        return BUTTON_NONE;
    }
}

enum button_t ButtonReadDown(void)
{
    static enum button_t prev_button = BUTTON_NONE;

    if (IoFromAvr.Buttons == 0)
        return prev_button = BUTTON_NONE;

    if (prev_button != BUTTON_NONE)
        return BUTTON_NONE;

    switch (IoFromAvr.Buttons)
    {
    case 127:
        return prev_button = BUTTON_LEFT;
    case 406:
        return prev_button = BUTTON_RIGHT;
    case 2047:
        return prev_button = BUTTON_ENTER;
    case 1023:
        return prev_button = BUTTON_EXIT;
    default:
        return prev_button = BUTTON_NONE;
    }
}
