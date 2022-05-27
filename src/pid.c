#include "pid.h"
#include "display.h"
#include "input.h"
#include "output.h"

#define MOTOR_OFFSET 0

float Kp = 13.62; // 13.62
float Ki = 2.89;  // 2.89
float Kd = 0.132; // 0.082
float dt = 0.002;

float proportional = 0;
float integral = 0;
float scale = 0.25;
float derivative = 0;
float proportional_prev = 0;

float u = 0;

void self_balance()
{
    proportional = InputReadGyro();
    integral += proportional * scale;
    derivative = (proportional - proportional_prev) / dt;

    u = Kp * proportional + Ki * integral + Kd * derivative;

    proportional_prev = proportional;

    // DisplayFloat(36, 8, u)   // FIXME

    if (u < -100 + MOTOR_OFFSET)
        u = -100 + MOTOR_OFFSET;
    else if (u > 100 - MOTOR_OFFSET)
        u = 100 - MOTOR_OFFSET;

    if (u < 0)
    {
        OutputSetSpeed(0, (int)u - MOTOR_OFFSET);
        OutputSetSpeed(2, (int)u - MOTOR_OFFSET);
    }
    else
    {
        OutputSetSpeed(0, (int)u + MOTOR_OFFSET);
        OutputSetSpeed(2, (int)u + MOTOR_OFFSET);
    }
}

void reset_self_balance()
{
    proportional = 0;
    integral = 0;
    derivative = 0;
    proportional_prev = 0;

    u = 0;
}
