#if !defined(_INDEPENDENT_TASK_H)
#define _INDEPENDENT_TASK_H

#include "sys.h"
#include "protocol.h"

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;
extern uint8_t RFID_x, RFID_y;
extern uint8_t RFID_DataBlockLoation;
extern uint8_t RFID_DataBuffer[17];


// 快速发送单个指令(次数和延时在宏传递的参数中定义)
#define LED_TimerStart() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStartTimer, 3, 20) // 开始计时
#define LED_TimerStop() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStopTimer, 3, 20)   // 停止计时
#define TFTPage_Next() Send_ZigBeeDataNTimes(ZigBee_TFTPageNext, 2, 100)           // TFT下一页


// 红外指令

#define Send_PlateToTFT()                             \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_1, 1, 600); \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_2, 1, 600)

void Save_StatusBeforeFoundRFID(void);
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue);

void Task_RFID_RoadSectionTrue(void);
void Task_RFID_RoadSectionFalse(void);

void TFT_Task(void);
void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);
void TFT_Task(void);
void QRCode_Task(uint8_t QRrequest);
void LEDDispaly_ShowDistance(uint16_t dis);
#define Ultrasonic_Task(times) Ultrasonic_GetAverage(times)
void StreetLight_AdjustTo(uint8_t targetLevel);
void BarrierGate_Task(uint8_t plate[6]);
void Voice_Task(void);
void RFID_Task(void);
void RotationLED_Plate(uint8_t plate[6], uint8_t coord[2]);

void TFT_Hex(uint8_t dat[3]);

bool Read_RFID_Block(uint8_t block, uint8_t *buf);

void Test_RFID(uint8_t block);

#endif // _INDEPENDENT_TASK_H
