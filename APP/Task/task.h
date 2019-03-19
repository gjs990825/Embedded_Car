#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "protocol.h"
#include "Timer.h"
#include "roadway_check.h"

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;

// 自动判断数据长度 _A(Auto)
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

// 快速发送单个指令(次数和延时在宏传递的参数中定义)
#define LED_TimerStart() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStartTimer, 3, 20) // 开始计时
#define LED_TimerStop() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStopTimer, 3, 20)   // 停止计时
#define TFTPage_Next() Send_ZigBeeDataNTimes(ZigBee_TFTPageNext, 2, 100)           // TFT下一页

// // 请求数据(或行为)
// #define Request_GarageFloor() Request_Data(RequestData_GarageFloor) // 请求车库层数
// #define Request_Infrared() Request_Data(RequestData_Infrared)       // 请求红外数据

// 红外指令

#define Send_PlateToTFT()                       \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_1, 1, 600); \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_2, 1, 600)

void Save_StatusBeforeFoundRFID(void);
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue);

void TFT_Task(void);
void QRCode_Task(void);
void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);
void TFT_Task(void);
void RotationLED_Task(void);
void QRCode_Task(void);
void LEDDispaly_ShowDistance(uint16_t dis);
#define Ultrasonic_Task(times) Ultrasonic_GetAverage(times)
void StreetLight_Task(uint8_t targetLevel);
void RFID_Task(void);
void BarrierGate_Task(uint8_t plate[6]);
void Voice_Task(void);

void Read_Card_Test(void);

// 任务点
void Task_5_5(void);
void Task_1_5(void);
void Task_3_5(void);
void Task_1_3(void);
void Task_5_3(void);
void Task_5_1(void);
void Task_3_1(void);

#endif // __TASK_H_
