#if !defined(_INDEPENDENT_TASK_H)
#define _INDEPENDENT_TASK_H

#include "sys.h"
#include "protocol.h"
#include "route.h"

enum
{
    TFT_A = 1,
    TFT_B = 2,
    TFT_ALL = 3,
};

typedef struct taskCoordinate
{
    uint8_t taskID;
    uint8_t *coord;
} taskCoord_t;

typedef struct DataToAGV_Strusct
{
    uint8_t *currentCoord;     // 当前坐标点
    uint8_t direction;         // 当前车头朝向
    uint8_t *routeInfo;        // 路径信息
    uint8_t *alarmData;        // 报警码
    taskCoord_t *taskCoord;    // 任务点和ID
    uint8_t taskNumber;        // 任务数量
    uint8_t *barrierGateCoord; // 道闸位置
    uint8_t *avoidGarage;      // 避让车库
    uint8_t *avoidGarage2;     // 备选避让车库
    uint8_t streetLightLevel;  // 路灯档位
    // uint8_t isBarrierChanged;  // 障碍点是否变更
} DataToAGV_t;

// RFID相关 ↓

// 单个数据块信息
typedef struct Block_Info_Struct
{
    int8_t block;     // 块位置, -1为空
    uint8_t authMode; // KEY类型
    uint8_t key[6];   // KEY
    uint8_t data[17]; // 数据
} Block_Info_t;

// RFID卡信息
typedef struct RFID_Info_Struct
{
    RouteNode_t coordinate;  // 卡片坐标
    Block_Info_t *blockInfo; // 数据块信息集合
    int8_t blockNumber;      // 数据块个数
} RFID_Info_t;

extern uint8_t FOUND_RFID_CARD;
extern uint8_t RFID_RoadSection;
extern RFID_Info_t *CurrentRFIDCard;

// 预设卡片信息

extern RFID_Info_t RFID1;
extern RFID_Info_t RFID2;
extern RFID_Info_t RFID3;

// 特殊地形相关 ↓

extern uint8_t Special_RoadSection;
extern uint8_t ENTER_SPECIAL_ROAD;
extern uint8_t Special_Road_Processed;

void Emergency_Flasher(uint16_t time);
void BEEP_Twice(void);

///////////////////////////////////////////////////////
// 下面是标志物的操作和控制函数
///////////////////////////////////////////////////////

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ RFID部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

#define RFID_RoadSectionTrue() RFID_RoadSection = true
#define RFID_RoadSectionFalse() RFID_RoadSection = false
void Save_RunningStatus(void);
void Resume_RunningStatus(uint16_t encoderChangeValue);
void Set_CurrentCardInfo(RFID_Info_t *RFIDx);
ErrorStatus Read_RFID(RFID_Info_t *RFIDx);
void RFID_Task(void);
void RFIDx_Begin(RFID_Info_t *RFIDx);
void RFIDx_End(void);

// RFID测试用函数

void RFID1_Begin(void);
void RFID2_Begin(void);
void RFID3_Begin(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 特殊地形部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void SpecialRoad_Task(void);
void SpecialRoad_Test(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 烽火台（报警台）部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void Alarm_ON(uint8_t code[6]);
void Alarm_ChangeCode(uint8_t code[6]);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 道闸部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void BarrierGate_Plate(uint8_t plate[6]);
void BarrierGate_Control(bool status);
void BarrierGate_ReturnStatus(void);
void BarrierGate_Task(uint8_t plate[6]);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ LED显示部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void LEDDisplay_DataToFistRow(uint8_t data[3]);
void LEDDisplay_DataToSecondRow(uint8_t data[3]);
void LEDDisplay_TimerMode(TimerMode_t mode);
void LEDDisplay_Distance(uint16_t dis);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体显示（旋转LED）部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void RotationLED_PlateAndCoord(uint8_t plate[6], RouteNode_t coord);
void RotationLED_Distance(uint8_t dis);
void RotationLED_Shape(Shape_t shape);
void RotationLED_Color(Color_t color);
void RotationLED_RouteStatus(RouteStatus_t status);
void RotationLED_Default(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ TFT显示器部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void TFT_ShowPicture(uint8_t TFTx, uint8_t picNumber);
void TFT_PicturePrevious(uint8_t TFTx);
void TFT_PictureNext(uint8_t TFTx);
void TFT_PictureAuto(uint8_t TFTx);
void TFT_Plate(uint8_t TFTx, uint8_t plate[6]);
void TFT_Timer(uint8_t TFTx, TimerMode_t mode);
void TFT_HexData(uint8_t TFTx, uint8_t data[3]);
void TFT_Distance(uint8_t TFTx, uint16_t dis);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体车库部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void StereoGarage_ToLayer(uint8_t garage_x, uint8_t layer);
void StereoGarage_ReturnLayer(uint8_t garage_x);
void StereoGarage_ReturnInfraredStatus(uint8_t garage_x);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 交通灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void TrafficLight_RecognitionMode(uint8_t light_x);
void TrafficLight_ConfirmColor(uint8_t light_x, TrafficLightColor_t light);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 语音识别部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

void VoiceBroadcast_Specific(uint8_t voiceID);
void VoiceBroadcast_Radom(void);
void VoiceRecognition_Return(uint8_t voiceID);
void Voice_Recognition(void);

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 无线充电部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

#define WirelessCharging_ON() Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 5, 100)
#define WirelessCharging_CodeON() Send_ZigBeeData(code)

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 智能路灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

uint8_t StreetLight_AdjustTo(uint8_t targetLevel);

///////////////////// 宏定义操作 /////////////////////

#define LEDDisplay_Timer(status) LEDDisplay_TimerMode(TimerMode_##status);

///////////////////////////////////////////////////////
// 下面是独立任务，进入前需保证位置距离朝向等准确无误
// 任务结束和开始车身方向不一样的需要手动设置 CurrentStatus.dir = DIR_XX;
///////////////////////////////////////////////////////

void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(uint8_t light_x);
void TFT_Task(uint8_t TFTx);
void QRCode_Task(uint8_t QRrequest);
void Voice_Task(void);
void ETC_Task(void);
void AGV_Task(DataToAGV_t agvData);

#endif // _INDEPENDENT_TASK_H
