#if !defined(_INDEPENDENT_TASK_H)
#define _INDEPENDENT_TASK_H

#include "sys.h"
#include "protocol.h"
#include "route.h"

// RFID相关 ↓

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;

typedef struct RFID_Info_Struct
{
    uint8_t dataBlockLocation;
    uint8_t authMode;
    RouteNode_t coordinate;
    uint8_t key[6];
    uint8_t data[17];
} RFID_Info_t;

#define RFID_RoadSectionTrue() RFID_RoadSection = true
#define RFID_RoadSectionFalse() RFID_RoadSection = false
void Save_StatusBeforeFoundRFID(void);
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue);
void Task_RFIDTestStart(void);
void Task_RFIDTestEnd(void);
void Set_CurrentCardInfo(RFID_Info_t *RFIDx);

// RFID相关 ↑

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

void Test_RFID(uint8_t block);

#endif // _INDEPENDENT_TASK_H
