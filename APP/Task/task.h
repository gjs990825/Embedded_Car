#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "protocol.h"

// øÏÀŸ÷∏¡Ó
#define LED_TimerStart() Send_ZigBeeData(ZigBee_LEDDisplayStartTimer, 3, 20)
#define LED_TimerStop() Send_ZigBeeData(ZigBee_LEDDisplayStopTimer, 3, 20)
#define TFTPage_Next() Send_ZigBeeData(ZigBee_TFTPageNext, 2, 100)




void TFT_Task(void);

void QRCode_Task(void);

void Start_Task(void);
void End_Task(void);

void LED_Timer(bool status);

#endif // __TASK_H_
