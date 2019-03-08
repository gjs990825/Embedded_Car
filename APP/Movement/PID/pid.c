#include "pid.h"
#include "my_lib.h"

// float Kp = 13, Ki = 0.09, Kd = 60;
float Kp = 20, Ki = 0.04, Kd = 20;
float error = 0, P = 0, I = 0, D = 0, PID_value = 0;
float previous_error = 0, previous_I = 0;

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

    constrain_float(D, -100, 100); // »ý·ÖÏÞ·ù

    PID_value = (Kp * P) + (Ki * I) + (Kd * D);
    PID_value = constrain_float(PID_value, -150, 150);

    previous_error = error;
}
