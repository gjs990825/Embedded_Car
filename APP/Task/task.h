#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "protocol.h"
#include "Timer.h"
#include "roadway_check.h"

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;

// �Զ��ж����ݳ��� _A(Auto)
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

// ���ٷ��͵���ָ��(��������ʱ�ں괫�ݵĲ����ж���)
#define LED_TimerStart() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStartTimer, 3, 20) // ��ʼ��ʱ
#define LED_TimerStop() Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStopTimer, 3, 20)   // ֹͣ��ʱ
#define TFTPage_Next() Send_ZigBeeDataNTimes(ZigBee_TFTPageNext, 2, 100)           // TFT��һҳ

// // ��������(����Ϊ)
// #define Request_GarageFloor() Request_Data(RequestData_GarageFloor) // ���󳵿����
// #define Request_Infrared() Request_Data(RequestData_Infrared)       // �����������

// ����ָ��

#define Send_PlateToTFT()                             \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_1, 1, 600); \
    Send_ZigBeeDataNTimes(ZigBee_PlateTFT_2, 1, 600)

void Save_StatusBeforeFoundRFID(void);
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue);

void TFT_Task(void);
void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);
void TFT_Task(void);
void QRCode_Task(uint8_t QRrequest);
void LEDDispaly_ShowDistance(uint16_t dis);
#define Ultrasonic_Task(times) Ultrasonic_GetAverage(times)
void StreetLight_Task(uint8_t targetLevel);
void BarrierGate_Task(uint8_t plate[6]);
void Voice_Task(void);
void RFID_Task(void);

void TFT_Hex(uint8_t dat[3]);

bool Read_RFID_Block(uint8_t block, uint8_t *buf);

void Task_5_1(void);
void Task_3_1(void);
void Task_1_3(void);
void Task_1_5(void);
void Task_3_5(void);
void Task_5_5(void);
void Task_3_1_2(void);
void Task_5_1_2(void);

// ����
void Task_5_1_Test(void);
void Task_3_3_Test(void);


void Test_1(uint8_t block);

// // �����
// void Task_5_5(void);
// void Task_1_5(void);
// void Task_3_5(void);
// void Task_1_3(void);
// void Task_5_3(void);
// void Task_5_1(void);
// void Task_3_1(void);

#endif // __TASK_H_
