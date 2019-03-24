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
#include "bh1750.h"

#define Action_S1() AGV_SetTaskID(1, 0)      // print_info("Plate:%s\r\n", Get_PlateNumber())
#define Action_S2() Test_1(7)   // LEDDispaly_ShowDistance(Ultrasonic_Task(10))
#define Action_S3() Test_1(5)              // AGV_SetRoute("G4F4F6D6D4D2F2G2")
#define Action_S4() Test_1(4) // AGV_SendInfraredData(Infrared_AlarmON)

// µ˜ ‘≈‰÷√
// #define Action_S1() print_info("Plate:%s\r\n", Get_PlateNumber())
// #define Action_S2() LEDDispaly_ShowDistance(Ultrasonic_Task(20))
// #define Action_S3() print_info("light:%d\r\n", BH1750_GetAverage(10))
// #define Action_S4() AGV_SendInfraredData(Infrared_AlarmON)

// ∞¥º¸≈‰÷√
#define S1 PIin(4)
#define S2 PIin(5)
#define S3 PIin(6)
#define S4 PIin(7)

// LED≈‰÷√
#define LED1 PHout(12)
#define LED2 PHout(13)
#define LED3 PHout(14)
#define LED4 PHout(15)

//∑‰√˘∆˜
#define MP_SPK PHout(5) 

void Cba_Init(void);
void Beep(uint8_t times);
void KEY_Check(void);

#endif
