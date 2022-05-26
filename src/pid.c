#include "pid.h"
#include "display.h"
#include "input.h"
#include "output.h"

#define MOTOR_OFFSET 0

float Kp = 786;   // 0.0336
float Ki = 2688;  // 0.2688
float Kd = 10.47; // 0.000504
float dt = 0.002;

float proportional = 0;
float integral = 0;
float scale = 0.25;
float derivative = 0;
float derivative_prev = 0;

float u = 0;

void self_balance()
{
    proportional = InputReadGyro();
    integral += derivative / scale;
    derivative = (derivative - derivative_prev) / dt;

    u = Kp * derivative + Ki * integral + Kd * proportional;

    derivative_prev = derivative;

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
    angle = 0;

    proportional = 0;
    integral = 0;
    derivative = 0;
    derivative_prev = 0;

    u = 0;
}
