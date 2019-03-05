#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"


void LED_Timer(bool status);
void LED_TimerStart(void);
void LED_TimerStop(void);


void Test_Task_1(void);
void Test_Task_2(void);
void Test_Task_3(void);

#endif // __TASK_H_
