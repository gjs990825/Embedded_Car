#include "stm32f4xx.h"
#include "sys.h"

#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

// ³¬Éù²¨Òý½Å¶¨Òå
#define INC PAout(15)

// ³¬Éù²¨Îó²î
static const float UltrasonicErrorValue = 0.0;

void Ultrasonic_Init(void);
void Ultrasonic_Ranging(void);
uint16_t Ultrasonic_GetAverage(uint8_t times);

extern uint16_t distance;

#endif




