#include "sys.h"

#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

// 超声波引脚定义
#define INC PAout(15)

// 超声波误差修正
// warning：在确定超声波数值变化基本线性时调整此数值
static const int UltrasonicErrorValue = 40;

void Ultrasonic_Init(void);
void Ultrasonic_Ranging(void);
uint16_t Ultrasonic_GetAverage(uint8_t times);

#endif
