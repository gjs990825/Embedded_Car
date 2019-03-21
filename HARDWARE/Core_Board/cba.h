#ifndef __CBA_H
#define __CBA_H

#include "stm32f4xx.h"
#include "sys.h"

// ∞¥º¸…Ë÷√∆Ù∂Ø≥Ã–Ú

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
#include "data_from_host.h"
#include "debug.h"

#define Action_S1() Auto_Run() //print_info("Plate:%s\r\n", Get_PlateNumber())
#define Action_S2() LEDDispaly_ShowDistance(Ultrasonic_Task(20)) // LEDDispaly_ShowDistance(Ultrasonic_Task(10))
#define Action_S3() print_info("ShapeINFO:%s\r\n", Get_ShapeInfo()) //Voice_Task() //SYN7318_Test()
#define Action_S4() print_info("AllColor:%d\r\n", Get_AllColorCount()) // AGV_SendInfraredData(Infrared_AlarmON)


// ∞¥º¸≈‰÷√

#define S1 PIin(4)
#define S2 PIin(5)
#define S3 PIin(6)
#define S4 PIin(7)

#define LED1 PHout(12)
#define LED2 PHout(13)
#define LED3 PHout(14)
#define LED4 PHout(15)

#define MP_SPK PHout(5) //∑‰√˘∆˜

void Cba_Init(void);
void Beep(uint8_t times);
void KEY_Check(void);

#endif
