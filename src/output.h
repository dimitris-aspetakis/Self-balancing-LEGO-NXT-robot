#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#define MAX_SPEED_FW (100)
#define MAX_SPEED_RW (-MAX_SPEED_FW)

#include <stdconst.h>

void OutputInit(void);
void OutputExit(void);
void OutputSetSpeed(UBYTE MotorNr, SBYTE Speed);

#endif
