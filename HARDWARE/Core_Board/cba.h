#ifndef __CBA_H
#define __CBA_H

#include "stm32f4xx.h"
#include "sys.h"

// ����������������

#include "infrared.h"
#include "data_base.h"
#include "syn7318.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"

#define Action_S1() autorun()// Infrared_Send(HW_K, 6) //�򿪲��Ժ��ⱨ��
#define Action_S2() Infrared_Send(H_1, 4)  //����
#define Action_S3() SYN7318_Test()
#define Action_S4() Read_Card()


// ��������

#define S1 PIin(4)
#define S2 PIin(5)
#define S3 PIin(6)
#define S4 PIin(7)

#define LED1 PHout(12)
#define LED2 PHout(13)
#define LED3 PHout(14)
#define LED4 PHout(15)

#define MP_SPK PHout(5) //������

void Cba_Init(void);
void Beep(uint8_t times);
void KEY_Check(void);

#endif
