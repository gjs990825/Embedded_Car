#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"

#define ExcuteNTimes(task, N)       \
    for (uint8_t i = 0; i < N; i++) \
    {                               \
        task;                       \
    }


void Start_Task(void);
void End_Task(void);
void TFT_Task(void);

void QRCode_Task(void);

void LED_Timer(bool status);
void LED_TimerStart(void);
void LED_TimerStop(void);


void Test_Task_1(void);
void Test_Task_2(void);
void Test_Task_3(void);

#endif // __TASK_H_
