#include "sys.h"

#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

// ���������Ŷ���
#define INC PAout(15)

// ���������
static const int UltrasonicErrorValue = 40;

void Ultrasonic_Init(void);
void Ultrasonic_Ranging(void);
uint16_t Ultrasonic_GetAverage(uint8_t times);

// extern uint16_t distance;

#endif

