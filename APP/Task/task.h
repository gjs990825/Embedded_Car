#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "independent_task.h"


void Task_A2(void);
void Task_B2(void);
void Task_B3(void);
void Task_B4(void);
void Task_D4(void);
void Task_F4(void);
void Task_F6(void);
void Task_D6(void);

void Task_Test(void);

void RFID1_Begin(void);
void RFID1_End(void);
void RFID2_Begin(void);
void RFID2_End(void);
void RFID3_Begin(void);
void RFID3_End(void);

#endif // __TASK_H_
