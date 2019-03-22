#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "canp_hostcom.h"
#include "uart_a72.h"
#include "delay.h"

// // 定义连接模式
// #define CONNECTION_MODE CONNECTION_WIFI
// // 使用宏定义的函数（程序可能会增大）
// #define USE_MACRO_FUNCTIONS false

// // 连接方式
// enum
// {
//     CONNECTION_SERIAL = 0,
//     CONNECTION_WIFI
// };

// 上传数据包的结构
enum
{
    Pack_Header1 = 0,
    Pack_Header2,
    Pack_MainCmd,
    Pack_SubCmd1,
    Pack_SubCmd2,
    Pack_SubCmd3,
    Pack_CheckSum,
    Pack_Ending
};

// 上位机发送的数据标识
enum
{
    // 官方文档内指令
    FromHost_Stop = 0x01,                       // 停止
    FromHost_Go = 0x02,                         // 前进
    FromHost_Back = 0x03,                       // 后退
    FromHost_TurnLeft = 0x04,                   // 左转
    FromHost_TurnRight = 0x05,                  // 右转
    FromHost_TrackLine = 0x06,                  // 循迹
    FromHost_EncoderClear = 0x07,               // 码盘清零
    FromHost_TurnCountClockWiseToDigree = 0x08, // 左转弯--角度
    FromHost_TurnClockWiseToDigree = 0x09,      // 右转弯--角度
    FromHost_InfraredFrontData = 0x10,          // 红外前三位数据
    FromHost_InfraredBackData = 0x11,           // 红外后三位数据
    FromHost_InfraredSend = 0x12,               // 通知小车单片机发送红外线
    FromHost_TurnningLightControl = 0x20,       // 转向灯控制
    FromHost_Beep = 0x30,                       // 蜂鸣器
    FromHost_NotUsed = 0x40,                    // 暂未使用
    FromHost_InfraredPhotoPrevious = 0x50,      // 红外发射控制相片上翻
    FromHost_InfraredPhotoNext = 0x51,          // 红外发射控制相片下翻
    FromHost_InfraredLightAdd1 = 0x61,          // 红外发射控制光源强度档位加1
    FromHost_InfraredLightAdd2 = 0x62,          // 红外发射控制光源强度档位加2
    FromHost_InfraredLightAdd3 = 0x63,          // 红外发射控制光源强度档位加3
    FromHost_AGVReturnData = 0x80,              // 从车返回数据
    FromHost_VoiceRecognition = 0x90,           // 语音识别
    // 自定义的指令
    FromHost_AGVOpenMvQRCode = 0x92,          // OpenMv二维码识别
    FromHost_AGVStart = 0xD0,                 // 从车启动
    FromHost_LEDDisplaySecomdRow = 0xc1,      // 数码管第二排显示是数据
    FromHost_ReceivePresetHeadTowards = 0x71, // 接收预案车头设置
    FromHost_Start = 0xA1,                    // 小车启动命令
    FromHost_QRCodeRecognition = 0xA2,        // 二维码识别
    FromHost_PlateRecognition = 0xA3,         // 车牌识别
    FromHost_ShapeRecongnition = 0xA4,        // 图像识别
    FromHost_TrafficLight = 0xA5,             // 交通灯
    FromHost_StreetLight = 0xA6,              // 路灯
    FromHost_AlarmON = 0xA7,                  // 报警器开
    FromHost_AlarmOFF = 0xA8,                 // 报警器关
    FromHost_PlateData1 = 0x88,               // 车牌信息1
    FromHost_PlateData2 = 0x99,               // 车牌信息2
    FromHost_Garage = 0xB1,                   // 立体车库
    FromHost_TFTRecognition = 0xAC,           // TFT识别

    // 从车专有的指令
    FromHost_AGVRouting = 0xA9, // AGV 接收路径设置
};

// ZigBee 返回数据头
enum
{
    Return_BarrierGate = 0x03, // 道闸的返回数据
    Return_ETC = 0x0C,         // ETC的返回数据
    Return_AGVComplete = 0x66, // 从车入库完成
};

// ZigBee回传的数据状态和时间戳
typedef struct ZigBee_DataStatus_Sturuct
{
    uint8_t isSet;
    uint32_t timeStamp;
} ZigBee_DataStatus_t;

#define Send_ToHost Send_WifiData_To_Fifo

// // 自适应选择发送函数
// #if CONNECTION_MODE == CONNECTION_SERIAL
// #define Send_ToHost Send_DataToUsart // 函数声明在下面
// #else
// #define Send_ToHost Send_WifiData_To_Fifo
// #endif // CONNECTION_MODE

#define GetCmdFlag(id) CommandFlagStatus[id]

// 执行N次，带延迟
#define ExcuteNTimes(Task, N, delay)    \
    do                                  \
    {                                   \
        for (uint8_t i = 0; i < N; i++) \
        {                               \
            Task;                       \
            delay_ms(delay);            \
        }                               \
    } while (0)

// void ExcuteNTimes(void(Task *)(void), N, delay);

void Send_ZigBeeData(uint8_t *data);
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay);
void Request_ToHost(uint8_t request);
void Request_Data(uint8_t dataRequest[2]);

/***************************************请求命令 Request_XX**************************************************/
// 上位机没有进行数据校验，校验和(Request_ToHostArray[Pack_CheckSum])无视

// 包头为 0x55, 0x03 ，包尾为 0xBB 的指令(请求上位机任务), Request_ToHostArray[Pack_MainCmd]替换为请求编号
enum
{
    RequestCmd_QRCode1 = 0x01,           // 二维码1
    RequestCmd_QRCode2 = 0x02,           // 二维码2
    RequestCmd_StreetLight = 0x31,       // 智能路灯
    RequestCmd_Garage = 0x05,            // 立体车库
    RequestCmd_TFTShow = 0x06,           // TFT显示
    RequestCmd_BarrierGate = 0x03,       // 道闸显示
    RequestCmd_LEDShow = 0x08,           // LED标志物显示
    RequestCmd_PlateRecognition = 0x11,  // 车牌识别
    RequestCmd_ShapeRecongnition = 0x21, // 图形识别
    RequestCmd_AGV = 0x07,               // AGV
    RequestCmd_Ultrasonic = 0x41,        // 超声波
    RequestCmd_Alarm = 0x51,             // 报警台
    RequestCmd_TrafficLight = 0x81,      // 交通灯
    RequestCmd_RotatingLED = 0x91,       // 旋转LED(立体显示)
    RequestCmd_TFTRecognition = 0x66,    // TFT识别
};

extern uint8_t RequestCmd_ToHostArray[];

// 包头为 0x55, 0x0D ，包尾为 0xBB 的指令(获取信息) 获取的信息在Can_WifiRx_Check中根据返回编号进行储存
static uint8_t RequestData_GarageFloor[2] = {0x02, 0x01}; // 请求车库位于第几层
static uint8_t RequestData_Infrared[2] = {0x02, 0x02};    // 请求红外

// 加一因为 0x00 未使用
#define DATA_REQUEST_NUMBER (12 + 1)

// 数据请求的包结构
enum
{
    Data_Header1 = 0, // 0x56
    Data_Header2,     // 0x66
    Data_RequestID,   // 请求ID
};

// 数据请求和返回
enum
{
    DataRequest_PlateNumber = 0x01, // 车牌号
    DataRequest_QRCode1,            // 二维码1
    DataRequest_QRCode2,            // 二维码2
    DataRequest_QRCodeSecondCar,    // 二维码3
    DataRequest_TrafficLight,       // 交通灯
    DataRequest_ShapeNumber,        // 形状数量
    DataRequest_ColorNumber,        // 颜色数量
    DataRequest_ShapeColorNumber,   // 形状颜色数量
    DataRequest_RFID,               // RFID数据
    DataRequest_Preset1,            // 预设1
    DataRequest_Preset2,            // 预设2
    DataRequest_Preset3,            // 预设3
};

// 定义每种数据的长度
enum
{
    DataLength_PlateNumber = 6,      // 车牌号
    DataLength_QRCode1 = 8,          // 二维码1
    DataLength_QRCode2 = 8,          // 二维码2
    DataLength_QRCodeSecondCar = 8,  // 二维码3
    DataLength_TrafficLight = 1,     // 交通灯
    DataLength_ShapeNumber = 1,      // 形状数量
    DataLength_ColorNumber = 1,      // 颜色数量
    DataLength_ShapeColorNumber = 1, // 形状颜色数量
    DataLength_RFID = 16,            // RFID数据
    DataLength_Preset1 = 16,         // 预设1
    DataLength_Preset2 = 16,         // 预设2
    DataLength_Preset3 = 16,         // 预设3
};

typedef struct DataSetting_Struct
{
    uint8_t *buffer;
    uint8_t Data_Length;
    uint8_t isSet;
} DataSetting_t;

extern DataSetting_t DataBuffer[DATA_REQUEST_NUMBER];

// 交通灯定义
enum
{
    TrafficLight_Red = 0x00,
    TrafficLight_Yellow,
    TrafficLight_Green,
};

// 形状定义
enum
{
    Shape_Triangle = 0x00, // 三角形
    Shape_Circle,          // 圆形
    Shape_Rectangle,       // 矩形
    Shape_Diamond,         // 菱形
    Shape_Pentagram,       // 五角星
};

// 颜色定义
enum
{
    Color_Red = 0, // 红
    Color_Green,   // 绿
    Color_Blue,    // 蓝
    Color_Yellow,  // 黄
    Color_Purple,  // 紫
    Color_Cyan,    // 青
    Color_Black,   // 黑
    Color_White,   // 白
};

/***************************************红外指令 Infrared_XX[X]**************************************************/
// 上位机无法直接发送
extern uint8_t Infrared_Data[6];
static uint8_t Infrared_PhotoPrevious[4] = {0x80, 0x7F, 0x05, ~(0x05)};     // 照片上翻
static uint8_t Infrared_PhotoNext[4] = {0x80, 0x7F, 0x1B, ~(0x1B)};         // 照片下翻
static uint8_t Infrared_LightAdd1[4] = {0x00, 0xFF, 0x0C, ~(0x0C)};         // 光源档位加1
static uint8_t Infrared_LightAdd2[4] = {0x00, 0xFF, 0x18, ~(0x18)};         // 光源档位加2
static uint8_t Infrared_LightAdd3[4] = {0x00, 0xFF, 0x5E, ~(0x5E)};         // 光源档位加3
static uint8_t Infrared_TunnelFanOn[4] = {0x00, 0xFF, 0x45, ~(0x45)};       // 隧道风扇系统打开
static uint8_t Infrared_AlarmON[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};  // 报警器打开
static uint8_t Infrared_AlarmOFF[6] = {0x67, 0x34, 0x78, 0xA2, 0xFD, 0x27}; // 报警器关闭
extern uint8_t Infrared_AlarmData[6];                                       // 报警器信息
// 立体显示
extern uint8_t Infrared_PlateData1[6]; // 车牌信息1
extern uint8_t Infrared_PlateData2[6]; // 车牌信息2
// LED显示（数码管）
extern uint8_t ZigBee_LEDDisplayDistanceData[8];

/***************************************ZigBee 数据 ZigBee_XX[8]**************************************************/
static uint8_t ZigBee_BarrierGateOPEN[8] = {0x55, 0x03, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};  // 道闸开启
static uint8_t ZigBee_BarrierGateCLOSE[8] = {0x55, 0x03, 0x01, 0x02, 0x00, 0x00, 0x03, 0xBB}; // 道闸关闭
static uint8_t ZigBee_TFTPagePrevious[8] = {0x55, 0x0b, 0x10, 0x01, 0x00, 0x00, 0x11, 0xbb};  // TFT向上翻页
static uint8_t ZigBee_TFTPageNext[8] = {0x55, 0x0b, 0x10, 0x02, 0x00, 0x00, 0x12, 0xbb};      // TFT向下翻页
static uint8_t ZigBee_TFTPageAuto[8] = {0x55, 0x0b, 0x10, 0x03, 0x00, 0x00, 0x13, 0xbb};      // TFT自动翻页

// 道闸显示车牌(未校验，原因暂时未知)
static uint8_t ZigBee_PlateBarrierGate_1[8] = {0x55, 0x03, 0x10, 0x43, 0x36, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_2[8] = {0x55, 0x03, 0x11, 0x38, 0x47, 0x31, 0x00, 0xBB};

// TFT显示车牌(未校验，原因暂时未知)
static uint8_t ZigBee_PlateTFT_1[8] = {0x55, 0x0b, 0x20, 0x41, 0x31, 0x32, 0xC4, 0xBB}; // 手动校验
static uint8_t ZigBee_PlateTFT_2[8] = {0x55, 0x0b, 0x21, 0x33, 0x42, 0x34, 0xCA, 0xBB}; // 手动校验

// 交通灯
static uint8_t ZigBee_TrafficLightStartRecognition[8] = {0x55, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x01, 0xBB}; //进入识别模式

// 无线充电
static uint8_t ZigBee_WirelessChargingON[8] = {0x55, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB}; //开启无线充电站

// LED显示标志物
extern uint8_t ZigBee_LEDDisplayData[8];                                                          // 数码管显示数据
static uint8_t ZigBee_LEDDisplayStartTimer[8] = {0x55, 0x04, 0x03, 0x01, 0x00, 0x00, 0x04, 0xBB}; // 数码管开始计时
static uint8_t ZigBee_LEDDisplayStopTimer[8] = {0x55, 0x04, 0x03, 0x00, 0x00, 0x00, 0x03, 0xBB};  // 数码管关闭计时
static uint8_t ZigBee_LEDDisplayDistance[8] = {0x55, 0x04, 0x04, 0x00, 0x02, 0x00, 0x06, 0xBB};   // 数码管显示距离

// 语音播报指令
static uint8_t ZigBee_VoiceRandom[8] = {0x55, 0x06, 0x20, 0x01, 0x00, 0x00, 0x00, 0xBB};         // 随机播报语音指令
static uint8_t ZigBee_VoiceTurnRight[8] = {0x55, 0x06, 0x10, 0x02, 0x00, 0x00, 0x12, 0xBB};      // 向右转弯
static uint8_t ZigBee_VoiceNOTurnRight[8] = {0x55, 0x06, 0x10, 0x03, 0x00, 0x00, 0x13, 0xBB};    // 禁止右转
static uint8_t ZigBee_VoiceDriveLeft[8] = {0x55, 0x06, 0x10, 0x04, 0x00, 0x00, 0x14, 0xBB};      // 左侧行驶
static uint8_t ZigBee_VoiceNODriveLeft[8] = {0x55, 0x06, 0x10, 0x05, 0x00, 0x00, 0x15, 0xBB};    // 左行被禁
static uint8_t ZigBee_VoiceTurnAround[8] = {0x55, 0x06, 0x10, 0x06, 0x00, 0x00, 0x16, 0xBB};     // 原地掉头
static uint8_t ZigBee_VoiceDriveAssistant[8] = {0x55, 0x06, 0x10, 0x01, 0x00, 0x00, 0x11, 0xBB}; // 驾驶助手

// 从车指令
extern uint8_t ZigBee_AGVStart[8];                                                         // 从车启动命令
extern uint8_t ZigBee_AGVPreset[8];                                                        // 从车预案
static uint8_t ZigBee_AGVOpenMV[8] = {0x55, 0x02, 0x92, 0x01, 0x00, 0x00, 0x00, 0xBB};     // 启动从车二维码识别
static uint8_t ZigBee_AGVTurnLED[8] = {0x55, 0x02, 0x20, 0x01, 0x01, 0x00, 0x00, 0xBB};    // 从车转向灯
static uint8_t ZigBee_GarageLayers1[8] = {0x55, 0x0D, 0x01, 0x01, 0x00, 0x00, 0x00, 0xBB}; // 停到1层
static uint8_t ZigBee_GarageLayers2[8] = {0x55, 0x0D, 0x01, 0x02, 0x00, 0x00, 0x00, 0xBB}; // 停到2层
static uint8_t ZigBee_GarageLayers3[8] = {0x55, 0x0D, 0x01, 0x03, 0x00, 0x00, 0x00, 0xBB}; // 停到3层
static uint8_t ZigBee_GarageLayers4[8] = {0x55, 0x0D, 0x01, 0x04, 0x00, 0x00, 0x00, 0xBB}; // 停到4层

// 当前指令状态和数据内容存放(指令不连续和标志位使用造成的空间浪费暂时未解决)
extern uint8_t CommandFlagStatus[0xFF];

void Send_DataToUsart(uint8_t *buf, uint32_t length);
void Check_Sum(uint8_t *cmd);

#endif // __PROTOCOL_H_
