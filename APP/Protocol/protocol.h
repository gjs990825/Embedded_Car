#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "delay.h"

// 与上位机通信的发送函数指针
extern void (*Send_ToHost)(uint8_t *, uint8_t);

// 定义连接模式
#define WIRED_CONNECTION 0    // 有线连接（串口）
#define WIRELESS_CONNECTION 1 // 无线连接（WIFI）

// 设定连接模式
// true无线，false有线
void SetConnectionMode(bool mode);

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
    FromHost_AGVRouting = 0xE1, // AGV 接收路径设置
    FromHost_AGVSetTask = 0xE2, // AGV 接收任务设置
    // AGV预留数据接口
    FromHost_AGVData1 = 0xEA, // AGV 数据接口1
    FromHost_AGVData2 = 0xEB, // AGV 数据接口2
    FromHost_AGVData3 = 0xEC, // AGV 数据接口3
    FromHost_AGVData4 = 0xED, // AGV 数据接口4
    FromHost_AGVData5 = 0xEE, // AGV 数据接口5
    FromHost_AGVData6 = 0xEF, // AGV 数据接口6
};

// ZigBee设备ID
// 返回数据第[Pack_Header2]字节数据
enum
{
    ZigBeeID_MainCar = 1,           // 主车
    ZigBeeID_AGV = 2,               // AGV运输车
    ZigBeeID_BarrierGate = 3,       // 道闸
    ZigBeeID_LEDDisplay = 4,        // LED显示
    ZigBeeID_StereoGarage_B = 5,    // 立体车库B
    ZigBeeID_VoiceBroadcast = 6,    // 语音播报
    ZigBeeID_Alarm = 7,             // 红外报警器
    ZigBeeID_TFTB = 8,              // TFT显示B
    ZigBeeID_StreetLight = 9,       // 自动调光
    ZigBeeID_WirelessCharging = 10, // (A) 无线充电
    ZigBeeID_TFTA = 11,             // (B) TFT显示A
    ZigBeeID_ETC = 12,              // (C) ETC系统
    ZigBeeID_StereoGarage_A = 13,   // (D) 立体车库A
    ZigBeeID_TrafficLight_A = 14,   // (E) 交通灯A
    ZigBeeID_TrafficLight_B = 15,   // (F) 交通灯B
    ZigBeeID_AutoJudgement = 175,   // (AF) 自动评分系统
};

// ZigBee返回名称
// 只借用了名称，没有数据
enum
{
    BarrierGate,    // 道闸
    ETC,            // ETC系统
    AGV,            // 从车返回
    VoiceBroadcast, // 语音播报
};

// 标志物和请求名称区分
enum
{
    TFT_A = 1,   // TFT A
    TFT_B = 2,   // TFT B
    TFT_ALL = 3, // TFT总和

    TrafficLight_A = 1, // 交通灯A
    TrafficLight_B = 2, // 交通灯B

    StereoGarage_A = 1, // 立体车库A
    StereoGarage_B = 2, // 立体车库B

    QRCode_1 = 1, // 二维码1
    QRCode_2 = 2, // 二维码2
};

// 通用ZigBee回传的数据状态和时间戳
typedef struct ZigBee_DataStatus_Sturuct
{
    uint8_t isSet;
    uint8_t cmd[8];
    uint32_t timeStamp;
} ZigBee_DataStatus_t;

// AGV预设任务
enum
{
    AGVPresetTask_AdjustBarrier = 9,  // 障碍点设置
    AGVPresetTask_QRCodeDistance = 8, // 扫描二维码并上传
    AGVPresetTask_Streetlight = 7,    // 调整路灯档位
};

// AGV预设数据
enum
{
    AGVPresetData_StreetLight = 0xEF, // 路灯档位
};

// 从车上传数据结构
enum
{
    AGVUploadData_Header1 = 0,  // 包头1
    AGVUploadData_Header2 = 1,  // 包头2
    AGVUploadData_DataType = 2, // 数据类型
};

// 从车上传数据类型
enum
{
    AGVUploadType_Ultrasonic = 0,       // 超声波
    AGVUploadType_Brightness = 1,       // 光照度
    AGVUploadType_QRCodeData = 0x92,    // 二维码数据
    AGVUploadType_MisonComplete = 0xFF, // 任务完成
};

// 任务请求的结构 [0][1] 包头（0x55, 0x03）[2] 任务ID

// 任务请求ID
enum
{
    RequestTask_QRCode1 = 0x01,         // 二维码1
    RequestTask_QRCode2 = 0x02,         // 二维码2
    RequestTask_TrafficLightA = 0x81,   // 交通灯A
    RequestTask_TrafficLightB = 0x82,   // 交通灯B
    RequestTask_TFTRecognitionA = 0x66, // TFT识别A
    RequestTask_TFTRecognitionB = 0x67, // TFT识别B
};

// 数据 发送/请求/返回 的包结构

// 发送 [0][1] 包头 [2] 发送ID [3] 数据长度 [4+] 数据区域
// 请求 [0][1] 包头 [2] 请求ID [3+] 附加参数区（用途/区分）
// 返回 [0][1] 包头 [2] 返回ID [3+] 数据区域
enum
{
    // 共用
    Data_Header1 = 0, // 头1
    Data_Header2 = 1, // 头2
    Data_ID = 2,      // ID

    // 非共用         // 定义-------|请求|发送|返回|
    Data_Use = 3,     // 数据用途位 | Y  |    |    |
    Data_Length = 3,  // 长度定义位 |    | Y  | Y  |
    Data_Content = 4, // 数据发送位 |    | Y  |    |
};

// 数据发送ID
enum
{
    DataSend_QRCode = 0, // 二维码
    DataSend_RFID = 1,   // RFID1
    DataSend_RFID2 = 2,  // RFID2
    DataSend_RFID3 = 3,  // RFID3
    DataSend_Preset1 = 4, // 预留1
    DataSend_Preset2 = 5, // 预留2
    DataSend_Preset3 = 6, // 预留3
};

// 数据请求和返回ID
enum
{
    DataRequest_NotUsed = 0,          // 未使用
    DataRequest_PlateNumber = 1,      // 车牌号
    DataRequest_QRCode1 = 2,          // 二维码1
    DataRequest_QRCode2 = 3,          // 二维码2
    DataRequest_QRCodeSecondCar = 4,  // 二维码3
    DataRequest_TrafficLight = 5,     // 交通灯
    DataRequest_ShapeNumber = 6,      // 形状数量
    DataRequest_ColorNumber = 7,      // 颜色数量
    DataRequest_ShapeColorNumber = 8, // 形状颜色数量
    DataRequest_RFID = 9,             // RFID数据
    DataRequest_TFTInfo = 10,         // TFT信息
    DataRequest_AllColorCount = 11,   // 颜色总和
    DataRequest_AllShapeCount = 12,   // 形状总和
    DataRequest_Preset1 = 13,         // 预设1
    DataRequest_Preset2 = 14,         // 预设2
    DataRequest_Preset3 = 15,         // 预设3
};

// 数据请求储存格式
typedef struct DataSetting_Struct
{
    uint8_t *buffer;     // 数据buffer
    uint8_t Data_Length; // 数据长度信息
    uint8_t isSet;       // 接收标志位
} DataSetting_t;

// 标志物协议的命令和功能字节

// 报警灯标志物
enum
{
    Alarm_CodeFront3Bytes = 0x10, // 报警码前三位
    Alarm_CodeBack3Bytes = 0x11   // 报警码后三位
};

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
    Color_PurpleMagenta,  // 紫（品）
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

// 下面的数据是基本不需要操作的固定数据，可直接发送

// 红外指令
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
static uint8_t ZigBee_VoiceRandom[8] = {0x55, 0x06, 0x20, 0x01, 0x00, 0x00, 0x00, 0xBB}; // 随机播报语音指令
// 下面时标志物默认指令，基本不用
static uint8_t ZigBee_VoiceTurnRight[8] = {0x55, 0x06, 0x10, 0x02, 0x00, 0x00, 0x12, 0xBB};      // 向右转弯
static uint8_t ZigBee_VoiceNOTurnRight[8] = {0x55, 0x06, 0x10, 0x03, 0x00, 0x00, 0x13, 0xBB};    // 禁止右转
static uint8_t ZigBee_VoiceDriveLeft[8] = {0x55, 0x06, 0x10, 0x04, 0x00, 0x00, 0x14, 0xBB};      // 左侧行驶
static uint8_t ZigBee_VoiceNODriveLeft[8] = {0x55, 0x06, 0x10, 0x05, 0x00, 0x00, 0x15, 0xBB};    // 左行被禁
static uint8_t ZigBee_VoiceTurnAround[8] = {0x55, 0x06, 0x10, 0x06, 0x00, 0x00, 0x16, 0xBB};     // 原地掉头
static uint8_t ZigBee_VoiceDriveAssistant[8] = {0x55, 0x06, 0x10, 0x01, 0x00, 0x00, 0x11, 0xBB}; // 驾驶助手

// 各个标志物的指令根据下面的模板填充后发送
// 标志物ZigBee包头为0xAA(固定)和0xXX(ZigBee编号)，包尾为0xBB
// 除自动评分终端外，其它ZigBee数据都需要校验后发送

// 基本格式：8字节，除自动评分不校验之外其它均遵循此规则
// [0] 0x55 包头 [1] ZigBee设备ID [2] 主指令 [3-5] 副指令 [6] 校验和 [7] 0xBB 包尾

static uint8_t ZigBee_AlarmData[8] = {0x55, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};        // 报警灯
static uint8_t ZigBee_BarrierGateData[8] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};  // 道闸
static uint8_t ZigBee_LEDDisplayData[8] = {0x55, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};   // LED显示
static uint8_t Infrared_RotationLEDData[6] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};            // 旋转LED
static uint8_t ZigBee_TFTData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};          // TFT显示器（A/B）
static uint8_t ZigBee_StereoGarageData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; // 立体车库（A/B）
static uint8_t ZigBee_TrafficLightData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; // 智能交通灯（A/B）
static uint8_t ZigBee_VoiceData[8] = {0x55, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};        // 语音播报
static uint8_t ZigBee_VoiceReturnData[8] = {0xAF, 0x06, 0x00, 0x02, 0x00, 0x00, 0x01, 0xBB};  // 语音返回自动评分终端

// 当前指令状态和数据内容存放(指令不连续和标志位使用造成的空间浪费暂时未解决)
extern uint8_t CommandFlagStatus[0xFF];

// 数据请求编号数量
extern uint8_t DATA_REQUEST_NUMBER;
extern DataSetting_t DataBuffer[];

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

// 自动判断数据长度
// warning: 传入参数不可为指针！
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

void Send_ZigBeeData(uint8_t *data);
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay);
void Send_DataToUsart(uint8_t *buf, uint8_t length);
void Check_Sum(uint8_t *cmd);

#endif // __PROTOCOL_H_
