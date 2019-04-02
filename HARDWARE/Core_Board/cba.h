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

// // ƒ¨»œ≈‰÷√
// #define Action_S1() Auto_Run()
// #define Action_S2() Start_VoiceCommandRecognition(3)
// #define Action_S3() Test_RFID(5)
// #define Action_S4() print_info("light:%d\r\n", BH1750_GetAverage(10))

// //  ˝æ›Ωªª•≈‰÷√
// #define Action_S1() print_info("Plate:%s\r\n", Get_PlateNumber())
// #define Action_S2() print_info("QRCOde:%s\r\n", Get_QRCode(DataRequest_QRCode1, 0))
// #define Action_S3() print_info("Shape:%d\r\n", Get_ShapeNumber(Shape_Triangle))
// #define Action_S4() print_info("AllColor:%d\r\n", Get_AllColorCount())

// // ¥”≥µ≤‚ ‘≈‰÷√
// #define Action_S1() AGV_SetTowards(DIR_UP)
// #define Action_S2() AGV_SetTaskID(1, 0)
// #define Action_S3() AGV_SetRoute("G4F4F6D6D4D2F2G2")
// #define Action_S4() AGV_SendInfraredData(Infrared_AlarmON)

// // ∞◊ø®µ˜ ‘≈‰÷√
// #define Action_S1() Test_RFID(7)
// #define Action_S2() Test_RFID(6)
// #define Action_S3() Test_RFID(5)
// #define Action_S4() Test_RFID(4)

// // »ŒŒÒ∞Âµ˜ ‘≈‰÷√
#define Action_S1() Infrared_Send_A(Infrared_AlarmON)
#define Action_S2() print_info("Diatance:%d\r\n", Ultrasonic_Task(20))
#define Action_S3() print_info("light:%d\r\n", BH1750_GetAverage(10))
#define Action_S4() Auto_Run()

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
