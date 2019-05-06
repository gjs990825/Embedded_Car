#if !defined(_INDEPENDENT_TASK_H)
#define _INDEPENDENT_TASK_H

#include "sys.h"
#include "protocol.h"
#include "route.h"

typedef struct DataToAGV_Strusct
{
    uint8_t *currentCoord;
    uint8_t direction;
    uint8_t *routeInfo;
    uint8_t *alarmData;
    uint8_t *taskCoord;
    uint8_t *barrierGateCoord;
} DataToAGV_t;

// RFID相关 ↓

typedef struct RFID_Info_Struct
{
    uint8_t dataBlockLocation;
    uint8_t authMode;
    RouteNode_t coordinate;
    uint8_t key[6];
    uint8_t data[17];
} RFID_Info_t;

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;
extern RFID_Info_t *CurrentRFIDCard;

void Emergency_Flasher(uint16_t time);

///////////////////////////////////////////////////////
// 下面是标志物的操作和控制函数
///////////////////////////////////////////////////////

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ RFID部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

#define RFID_RoadSectionTrue() RFID_RoadSection = true
#define RFID_RoadSectionFalse() RFID_RoadSection = false
void Save_StatusBeforeFoundRFID(void);
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue);
void Set_CurrentCardInfo(RFID_Info_t *RFIDx);
ErrorStatus Read_RFID(RFID_Info_t *RFIDx);
void RFID_Task(void);

// RFID测试用函数

void Task_RFIDTestStart(void);
void Task_RFIDTestEnd(void);
void Test_RFID(uint8_t block);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 道闸部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void BarrierGate_Plate(uint8_t plate[6]);
void BarrierGate_Control(bool status);
void BarrierGate_ReturnStatus(void);
void BarrierGate_Task(uint8_t plate[6]);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ LED显示部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void LEDDisplay_DataToFistRow(uint8_t data[3]);
void LEDDisplay_DataToSecondRow(uint8_t data[3]);
void LEDDisplay_TimerMode(TimerMode_t mode);
void LEDDisplay_Distance(uint16_t dis);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体显示（旋转LED）部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void RotationLED_PlateAndCoord(uint8_t plate[6], RouteNode_t coord);
void RotationLED_Distance(uint8_t dis);
void RotationLED_Shape(Shape_t shape);
void RotationLED_Color(Color_t color);
void RotationLED_RouteStatus(RouteStatus_t status);
void RotationLED_Default(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ TFT显示器部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void TFT_ShowPicture(uint8_t picNumber);
void TFT_PicturePrevious(void);
void TFT_PictureNext(void);
void TFT_PictureAuto(void);
void TFT_Plate(uint8_t plate[6]);
void TFT_Timer(TimerMode_t mode);
void TFT_HexData(uint8_t data[3]);
void TFT_Distance(uint16_t dis);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体车库部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void StereoGarage_ToLayer(uint8_t layer);
void StereoGarage_ReturnLayer(void);
void StereoGarage_ReturnInfraredStatus(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 交通灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void TrafficLight_RecognitionMode(void);
void TrafficLight_ConfirmColor(TrafficLightColor_t light);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 语音识别部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void VoiceBroadcast_Specific(uint8_t voiceID);
void VoiceBroadcast_Radom(void);
void VoiceRecognition_Return(uint8_t voiceID);
void Voice_Recognition(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 智能路灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void StreetLight_AdjustTo(uint8_t targetLevel);

/////////////////////// 宏定义操作 /////////////////////

#define WirelessCharging_ON() Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 5, 100)
#define LEDDisplay_Timer(status) LEDDisplay_TimerMode(TimerMode_##status);

///////////////////////////////////////////////////////
// 下面是独立任务，进入前需保证位置距离朝向等准确无误
// 任务结束和开始车身方向不一样的需要手动设置 CurrentStaus.dir = DIR_XX;
///////////////////////////////////////////////////////

void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);
void TFT_Task(void);
void QRCode_Task(uint8_t QRrequest);
void Voice_Task(void);

void AGV_Task(DataToAGV_t agvData);

#endif // _INDEPENDENT_TASK_H
