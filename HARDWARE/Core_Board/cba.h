#ifndef __CBA_H
#define __CBA_H

#include "stm32f4xx.h"
#include "sys.h"

// ����������������

#include "infrared.h"
#include "data_base.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "data_base.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"
#include "voice.h"

#define Action_S1() Auto_Run()// Infrared_Send(HW_K, 6) //�򿪲��Ժ��ⱨ��
#define Action_S2() Start_VoiceCommandRecognition(3) //LEDDispaly_ShowDistance(Ultrasonic_GetAverage(20)) // LEDDispaly_ShowDistance(Ultrasonic_Task(10)) //Infrared_Send(H_1, 4)  //����
#define Action_S3() Task_5_3() //SYN7318_Test() // SYN7318_Test()
#define Action_S4() Send_ZigbeeData_To_Fifo(ZigBee_TFTPageNext, 8) // AGV_SendInfraredData(Infrared_AlarmON) //Auto_Run() //Turn_ByEncoder(-360)


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
