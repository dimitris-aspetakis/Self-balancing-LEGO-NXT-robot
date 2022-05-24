#include "pid.h"
#include "display.h"
#include "input.h"
#include "output.h"

#define MOTOR_OFFSET 40

float Kp = 0.0436;   // 0.0336
float Ki = 0.71688;  // 0.2688
float Kd = 0.00504; // 0.000504
// float Ti = 0.02;
// float Td = 0.2;
float dt = 0.002;

float past_values[1000] = {0};
unsigned curr_index = 0;
float d = 0;
float e = 0;
float e_prev = 0;
float u = 0;

void self_balance()
{
    e = InputReadGyro();
    past_values[curr_index] = e;
    d = (e - e_prev) / dt;
    float sum = 0;
    for (unsigned i = 0; i < 1000; ++i)
        sum += past_values[i];
    u = Kp * e + Ki * sum + Kd * d;
    // u = Kp*(e + 1/Ti*i + Td*d);
    e_prev = e;

    // DisplayFloat(36, 8, u);

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

    curr_index = (curr_index + 1) % 1000;
}

void reset_self_balance()
{
    for (unsigned i = 0; i < 1000; ++i)
        past_values[i] = 0;
    d = 0;
    e = 0;
    e_prev = 0;
    u = 0;
}
