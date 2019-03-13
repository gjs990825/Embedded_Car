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
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "data_base.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"

#define Action_S1() ExcuteAndWait(Start_Tracking(Track_Speed), Stop_Flag, CROSSROAD) // Auto_Run()// Infrared_Send(HW_K, 6) //�򿪲��Ժ��ⱨ��
#define Action_S2() ExcuteAndWait(Go_Ahead(Track_Speed, ToCrossroadCenter), Stop_Flag, FORBACKCOMPLETE) // LEDDispaly_ShowDistance(Ultrasonic_Task(10)) //Infrared_Send(H_1, 4)  //����
#define Action_S3() AGV_TrackLine(50) //SYN7318_Test() // LEDDispaly_ShowDistance(233) // SYN7318_Test()
#define Action_S4() Auto_Run() //Turn_ByEncoder(-360) // StreetLight_Task(3) //Read_Card()


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
