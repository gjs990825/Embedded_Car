#include "pid.h"

float Kp = 13, Ki = 0.09, Kd = 60;
float error = 0, P = 0, I = 0, D = 0, PID_value = 0;
float previous_error = 0, previous_I = 0;

float constrain(float x, float a, float b)
{
    if ((x >= a) && (x <= b))
    {
        return x;
    }
    else
    {
        return (x < a) ? a : b;
    }
}

void PidData_Clear(void)
{
    error = 0, P = 0, I = 0, D = 0, PID_value = 0;
    previous_error = 0, previous_I = 0;
}

void Calculate_pid(int inputError)
{
    error = inputError;
    P = error;
    I = I + error;
    D = error - previous_error;

    PID_value = (Kp * P) + (Ki * I) + (Kd * D);
    PID_value = constrain(PID_value, -150, 150);

    previous_error = error;
}
