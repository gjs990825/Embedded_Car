#ifndef __CBA_H
#define __CBA_H

#include "sys.h"

// ��������
#define S1 PIin(4)
#define S2 PIin(5)
#define S3 PIin(6)
#define S4 PIin(7)

// LED����
#define LED1 PHout(12)
#define LED2 PHout(13)
#define LED3 PHout(14)
#define LED4 PHout(15)

//������
#define MP_SPK PHout(5)

void Cba_Init(void);
void Beep(uint8_t times);
void KEY_Check(void);

#endif
