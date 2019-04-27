#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "canp_hostcom.h"
#include "uart_a72.h"
#include "delay.h"

// 定义连接模式
#define WIRED_CONNECTION 0    // 有线连接（串口）
#define WIRELESS_CONNECTION 1 // 无线连接（WIFI）

// 选择与上位机的连接方式
#define CONNECTION_MODE WIRELESS_CONNECTION

// 自适应选择发送函数
#if CONNECTION_MODE == WIRED_CONNECTION

#define Send_ToHost Send_DataToUsart

#elif CONNECTION_MODE == WIRELESS_CONNECTION

#define Send_ToHost Send_WifiData_To_Fifo

#endif

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
    FromHost_AGVSetTask = 0xAA, // AGV 接收任务设置
};

// ZigBee返回名称(没有使用到数据，只借用了名称)
enum
{
    BarrierGate,    // 道闸
    ETC,            // ETC系统
    AGVComplete,    // 从车入库完成
    TrafficLight,   // 交通灯
    StereoGarage,   // 立体车库
    AGV,            // 从车返回
    VoiceBroadcast, // 语音播报
};

// ZigBee 返回数据头
enum
{
    Return_BarrierGate = 0x03,   // 道闸
    Return_ETC = 0x0C,           // ETC
    Return_AGVComplete = 0x66,   // 从车
    Return_TrafficLight = 0x0E,  // 交通灯
    Return_StereoGarage = 0x0D,  // 立体车库
    Return_AGV = 0x02,           // 从车
    Return_VoiceBroadcast = 0x06 // 语音
};

// ZigBee回传的数据状态和时间戳
typedef struct ZigBee_DataStatus_Sturuct
{
    uint8_t isSet;
    uint8_t cmd[8];
    uint32_t timeStamp;
} ZigBee_DataStatus_t;

#define GetCmdFlag(id) CommandFlagStatus[id]
#define SetCmdFlag(id) CommandFlagStatus[id] = SET
#define ResetCmdFlag(id) CommandFlagStatus[id] = RESET

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

// 自动判断数据长度（传入参数不可为指针！）
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

void Send_ZigBeeData(uint8_t *data);
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay);
void Request_ToHost(uint8_t request);
void Request_Data(uint8_t dataRequest[2]);

/***************************************请求命令 Request_XX**************************************************/
// 上位机没有进行数据校验，校验和(Request_ToHostArray[Pack_CheckSum])无视

// 包头为 0x55, 0x03 ，包尾为 0xBB 的指令(请求上位机任务), Request_ToHostArray[Pack_MainCmd]替换为请求编号
enum
{
    RequestCmd_QRCode1 = 0x01,        // 二维码1
    RequestCmd_QRCode2 = 0x02,        // 二维码2
    RequestCmd_TrafficLight = 0x81,   // 交通灯
    RequestCmd_TFTRecognition = 0x66, // TFT识别
};

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
    DataRequest_NotUsed = 0x00,   // 未使用
    DataRequest_PlateNumber,      // 车牌号
    DataRequest_QRCode1,          // 二维码1
    DataRequest_QRCode2,          // 二维码2
    DataRequest_QRCodeSecondCar,  // 二维码3
    DataRequest_TrafficLight,     // 交通灯
    DataRequest_ShapeNumber,      // 形状数量
    DataRequest_ColorNumber,      // 颜色数量
    DataRequest_ShapeColorNumber, // 形状颜色数量
    DataRequest_RFID,             // RFID数据
    DataRequest_Preset1,          // 预设1
    DataRequest_Preset2,          // 预设2
    DataRequest_Preset3,          // 预设3
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

// 数据请求储存格式
typedef struct DataSetting_Struct
{
    uint8_t *buffer;     // 数据buffer
    uint8_t Data_Length; // 数据长度信息
    uint8_t isSet;       // 接收标志位
} DataSetting_t;

extern uint8_t DATA_REQUEST_NUMBER;
extern DataSetting_t DataBuffer[];

// 道闸标志物
enum
{
    BarrierGateMode_Control = 0x01,          // 控制开关
    BarrierGateMode_PlateFront3Bytes = 0x10, // 车牌前三位
    BarrierGateMode_PlateBack3Bytes = 0x11,  // 车牌后三位
    BarrierGateMode_ReturnStatus = 0x20      // 状态返回
};

// LED显示标志物
enum
{
    LEDDisplayMainCmd_DataToFirstRow = 0x01, // 第一行显示数据
    LEDDisplayMainCmd_DataToSecondRow,       // 第二行显示数据
    LEDDisplayMainCmd_TimerMode,             // 计时模式
    LEDDisplayMainCmd_ShowDistance           // 显示距离
};

// 立体显示标志物
enum
{
    RotationLEDMode_PlateFront4BytesData = 0x20,        // 接收前四位车牌信息
    RotationLEDMode_PlateBack2BytesAndCoordInfo = 0x10, // 接收后两位车牌和两位坐标信息并显示
    RotationLEDMode_Distance = 0x11,                    // 显示距离
    RotationLEDMode_Shape = 0x12,                       // 显示图形
    RotationLEDMode_Color = 0x13,                       // 显示颜色
    RotationLEDMode_RouteStatus = 0x14,                 // 显示路况
    RotationLEDMode_Default = 0x15                      // 显示默认
};

// TFT显示器标志物
enum
{
    TFTMode_Picture = 0x10,    // 图片
    TFTMode_PlateDataA = 0x20, // 车牌数据A
    TFTMode_PlateDataB = 0x21, // 车牌数据B
    TFTMode_Timer = 0x30,      // 计时
    TFTMode_Hex = 0x40,        // HEX显示
    TFTMode_Distance = 0x50    // 距离显示（十进制）
};

// 立体车库标志物
enum
{
    StereoGarage_Control = 0x01, // 控制
    StereoGarage_Return = 0x02   // 返回
};

// 交通灯标志物
enum
{
    TrafficLight_Recognition = 0x01, // 进入识别模式
    TrafficLight_Confirm = 0x02      // 确认识别结果
};

// 语音播报标志物
enum
{
    VoiceCmd_Specific = 0x10, // 特定语音
    VoiceCmd_Random = 0x20    // 随机语音
};

// 计时控制
typedef enum
{
    TimerMode_OFF = 0x00, // 计时关
    TimerMode_ON,         // 计时开
    TimerMode_Clear       // 计时清零
} TimerMode_t;

// 交通灯定义
typedef enum
{
    TrafficLightColor_Red = 0x01,   // 红灯
    TrafficLightColor_Green = 0x02, // 绿灯
    TrafficLightColor_Yellow = 0x03 // 黄灯
} TrafficLightColor_t;

// 形状定义
typedef enum
{
    Shape_NotDefined = 0, // 未定义
    Shape_Rectangle,      // 矩形
    Shape_Circle,         // 圆形
    Shape_Triangle,       // 三角形
    Shape_Diamond,        // 菱形
    Shape_Trapezoid,      // 梯形
    Shape_Pie,            // 饼图
    Shape_Traget,         // 靶图
    Shape_Bar,            // 条形图
    Shape_Pentagram,      // 五角星
} Shape_t;

// 颜色定义
typedef enum
{
    Color_NotDefined = 0, // 未定义
    Color_Red,            // 红
    Color_Green,          // 绿
    Color_Blue,           // 蓝
    Color_Yellow,         // 黄
    Color_Purple,         // 紫
    Color_Cyan,           // 青
    Color_Black,          // 黑
    Color_White,          // 白
} Color_t;

// 路况定义
typedef enum
{
    RouteStatus_TunnelAccident = 0x01, // 隧道事故
    RouteStatus_Construction = 0x02    // 前方施工
} RouteStatus_t;

/***************************************红外指令 Infrared_XX[X]**************************************************/
// 上位机无法直接发送
static uint8_t Infrared_LightAdd1[4] = {0x00, 0xFF, 0x0C, ~(0x0C)};         // 光源档位加1
static uint8_t Infrared_LightAdd2[4] = {0x00, 0xFF, 0x18, ~(0x18)};         // 光源档位加2
static uint8_t Infrared_LightAdd3[4] = {0x00, 0xFF, 0x5E, ~(0x5E)};         // 光源档位加3
static uint8_t Infrared_AlarmON[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};  // 报警器打开
static uint8_t Infrared_AlarmOFF[6] = {0x67, 0x34, 0x78, 0xA2, 0xFD, 0x27}; // 报警器关闭

// 交通灯
static uint8_t ZigBee_TrafficLightStartRecognition[8] = {0x55, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x01, 0xBB}; //进入识别模式

// 无线充电
static uint8_t ZigBee_WirelessChargingON[8] = {0x55, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB}; //开启无线充电站

// 语音播报指令
static uint8_t ZigBee_VoiceRandom[8] = {0x55, 0x06, 0x20, 0x01, 0x00, 0x00, 0x00, 0xBB};         // 随机播报语音指令
static uint8_t ZigBee_VoiceTurnRight[8] = {0x55, 0x06, 0x10, 0x02, 0x00, 0x00, 0x12, 0xBB};      // 向右转弯
static uint8_t ZigBee_VoiceNOTurnRight[8] = {0x55, 0x06, 0x10, 0x03, 0x00, 0x00, 0x13, 0xBB};    // 禁止右转
static uint8_t ZigBee_VoiceDriveLeft[8] = {0x55, 0x06, 0x10, 0x04, 0x00, 0x00, 0x14, 0xBB};      // 左侧行驶
static uint8_t ZigBee_VoiceNODriveLeft[8] = {0x55, 0x06, 0x10, 0x05, 0x00, 0x00, 0x15, 0xBB};    // 左行被禁
static uint8_t ZigBee_VoiceTurnAround[8] = {0x55, 0x06, 0x10, 0x06, 0x00, 0x00, 0x16, 0xBB};     // 原地掉头
static uint8_t ZigBee_VoiceDriveAssistant[8] = {0x55, 0x06, 0x10, 0x01, 0x00, 0x00, 0x11, 0xBB}; // 驾驶助手

// 从车指令
static uint8_t ZigBee_AGVOpenMV[8] = {0x55, 0x02, 0x92, 0x01, 0x00, 0x00, 0x00, 0xBB};  // 启动从车二维码识别
static uint8_t ZigBee_AGVTurnLED[8] = {0x55, 0x02, 0x20, 0x01, 0x01, 0x00, 0x00, 0xBB}; // 从车转向灯

// 指令发送模板↓

// 道闸标志物数据
static uint8_t ZigBee_BarrierGateData[8] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

// LED显示标志物数据
static uint8_t ZigBee_LEDDisplayData[8] = {0x55, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

// 旋转LED标志物数据
static uint8_t Infrared_RotationLEDData[6] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};

// TFT显示器标志物数据
static uint8_t ZigBee_TFTData[8] = {0x55, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

// 立体车库标志物数据
static uint8_t ZigBee_StereoGarageData[8] = {0x55, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

// 智能路灯标志物数据
static uint8_t ZigBee_TrafficLightData[8] = {0x55, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x01, 0xBB};

// 语音播报标志物数据
static uint8_t ZigBee_VoiceData[8] = {0x55, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

// 语音返回自动评分终端数据
static uint8_t ZigBee_VoiceReturnData[8] = {0xAF, 0x06, 0x00, 0x02, 0x00, 0x00, 0x01, 0xBB};

// 当前指令状态和数据内容存放(指令不连续和标志位使用造成的空间浪费暂时未解决)
extern uint8_t CommandFlagStatus[0xFF];

void Send_DataToUsart(uint8_t *buf, uint32_t length);
void Check_Sum(uint8_t *cmd);

#endif // __PROTOCOL_H_
