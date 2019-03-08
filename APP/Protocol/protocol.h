#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "canp_hostcom.h"
#include "uart_a72.h"
#include "delay.h"

// 定义连接模式
#define CONNECTION_MODE CONNECTION_WIFI
// 使用宏定义的函数（程序可能会增大）
#define USE_MACRO_FUNCTIONS false

// 连接方式
enum
{
    CONNECTION_SERIAL = 0,
    CONNECTION_WIFI
};

// 上传数据包的结构
typedef enum
{
    Pack_Header1 = 0,
    Pack_Header2,
    Pack_MainCmd,
    Pack_SubCmd1,
    Pack_SubCmd2,
    Pack_SubCmd3,
    Pack_CheckSum,
    Pack_Ending
} DataPackUpLoad_t;

// 自适应选择发送函数
#if CONNECTION_MODE
#define Send_ToHost Send_DataToUsart // 函数声明在下面
#else
#define Send_ToHost Send_WifiData_To_Fifo
#endif // CONNECTION_MODE

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

#if USE_MACRO_FUNCTIONS

// 发送N次ZigBee数据（默认八位数据）
#define Send_ZigBeeData(data, ntimes, delay)                           \
    do                                                                 \
    {                                                                  \
        ExcuteNTimes(Send_ZigbeeData_To_Fifo(data, 8), ntimes, delay); \
    } while (0)

// 填充请求并发送
#define Request_ToHost(request)                      \
    do                                               \
    {                                                \
        Request_ToHostArray[Pack_MainCmd] = request; \
        Send_ToHost(Request_ToHostArray, 8);         \
    } while (0)

#define Request_Data(dataRequest)                           \
    do                                                      \
    {                                                       \
        Request_ToHostArray[Pack_MainCmd] = dataRequest[0]; \
        Request_ToHostArray[Pack_SubCmd1] = dataRequest[1]; \
        Send_ToHost(Request_ToHostArray, 8);                \
    } while (0);

#else // USE_MACRO_FUNCTIONS

// void ExcuteNTimes(void(Task *)(void), N, delay);

void Send_ZigBeeData(uint8_t *data, uint8_t ntimes, uint16_t delay);
void Request_ToHost(uint8_t request);
void Request_Data(uint8_t dataRequest[2]);

#endif // USE_MACRO_FUNCTIONS

/***************************************请求命令 Request_XX**************************************************/
// 上位机没有进行数据校验，校验和(Request_ToHostArray[Pack_CheckSum])无视

// 包头为 0x55, 0x03 ，包尾为 0xBB 的指令(请求上位机任务), Request_ToHostArray[Pack_MainCmd]替换为请求编号
extern uint8_t RequestCmd_ToHostArray[];
static uint8_t RequestCmd_QRCode1 = 0x01;           // 二维码1
static uint8_t RequestCmd_QRCode2 = 0x02;           // 二维码2
static uint8_t RequestCmd_StreetLight = 0x31;       // 智能路灯
static uint8_t RequestCmd_Garage = 0x05;            // 立体车库
static uint8_t RequestCmd_TFTShow = 0x06;           // TFT显示
static uint8_t RequestCmd_BarrierGate = 0x03;       // 道闸显示
static uint8_t RequestCmd_LEDShow = 0x08;           // LED标志物显示
static uint8_t RequestCmd_PlateRecognition = 0x11;  // 车牌识别
static uint8_t RequestCmd_ShapeRecongnition = 0x21; // 图形识别
static uint8_t RequestCmd_AGV = 0x07;               // AGV
static uint8_t RequestCmd_Ultrasonic = 0x41;        // 超声波
static uint8_t RequestCmd_Alarm = 0x51;             // 报警台
static uint8_t RequestCmd_TrafficLight = 0x81;      // 交通灯
static uint8_t RequestCmd_RotatingLED = 0x91;       // 旋转LED(立体显示)

// 包头为 0x55, 0x0D ，包尾为 0xBB 的指令(获取信息) 获取的信息在Can_WifiRx_Check中根据返回编号进行储存
static uint8_t RequestData_GarageFloor[2] = {0x02, 0x01}; // 请求车库位于第几层
static uint8_t RequestData_Infrared[2] = {0x02, 0x02};    // 请求红外


/***************************************红外指令 Infrared_XX[X]**************************************************/
// 上位机无法直接发送
static uint8_t Infrared_PhotoPrevious[4] = {0x80, 0x7F, 0x05, ~(0x05)};       // 照片上翻
static uint8_t Infrared_PhotoNext[4] = {0x80, 0x7F, 0x1B, ~(0x1B)};           // 照片下翻
static uint8_t Infrared_LightAdd1[4] = {0x00, 0xFF, 0x0C, ~(0x0C)};           // 光源档位加1
static uint8_t Infrared_LightAdd2[4] = {0x00, 0xFF, 0x18, ~(0x18)};           // 光源档位加2
static uint8_t Infrared_LightAdd3[4] = {0x00, 0xFF, 0x5E, ~(0x5E)};           // 光源档位加3
static uint8_t Infrared_TunnelFanOn[4] = {0x00, 0xFF, 0x45, ~(0x45)};         // 隧道风扇系统打开
static uint8_t Infrared_AlarmON[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};    // 报警器打开
static uint8_t Infrared_AlarmOFF[6] = {0x67, 0x34, 0x78, 0xA2, 0xFD, 0x27};   // 报警器关闭
// 立体显示
static uint8_t Infrared_PlateData1[6] = {0xFF, 0x20, 0x41, 0x31, 0x42, 0x32}; // 车牌信息1
static uint8_t Infrared_PlateData2[6] = {0xFF, 0x10, 0x43, 0x35, 0x45, 0x31}; // 车牌信息2


/***************************************ZigBee 数据 ZigBee_XX[8]**************************************************/
static uint8_t ZigBee_BarrierGateOPEN[8] = {0x55, 0x03, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};  // 道闸开启
static uint8_t ZigBee_BarrierGateCLOSE[8] = {0x55, 0x03, 0x01, 0x02, 0x00, 0x00, 0x03, 0xBB}; // 道闸关闭
static uint8_t ZigBee_TFTPagePrevious[8] = {0x55, 0x0b, 0x10, 0x01, 0x00, 0x00, 0x11, 0xbb};  // TFT向上翻页
static uint8_t ZigBee_TFTPageNext[8] = {0x55, 0x0b, 0x10, 0x02, 0x00, 0x00, 0x12, 0xbb};      // TFT向下翻页
static uint8_t ZigBee_TFTPageAuto[8] = {0x55, 0x0b, 0x10, 0x03, 0x00, 0x00, 0x13, 0xbb};      // TFT自动翻页

// 暂未使用
// static uint8_t CP_G1[6] = {0xFF, 0x12, 0x01, 0x00, 0x00, 0x00};
// static uint8_t CP_G2[6] = {0xFF, 0x13, 0x01, 0x00, 0x00, 0x00};

// 道闸显示车牌(未校验，原因暂时未知)
static uint8_t ZigBee_PlateBarrierGate_1[8] = {0x55, 0x03, 0x10, 0x43, 0x36, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_2[8] = {0x55, 0x03, 0x11, 0x38, 0x47, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test1[8] = {0x55, 0x03, 0x10, 0x5a, 0x37, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test2[8] = {0x55, 0x03, 0x11, 0x36, 0x4b, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test3[8] = {0x55, 0x03, 0x10, 0x46, 0x31, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test4[8] = {0x55, 0x03, 0x11, 0x39, 0x54, 0x33, 0x00, 0xBB};

// TFT显示车牌(未校验，原因暂时未知)
static uint8_t ZigBee_PlateTFT_1[8] = {0x55, 0x0b, 0x20, 0x41, 0x31, 0x32, 0xC4, 0xBB}; // 手动校验
static uint8_t ZigBee_PlateTFT_2[8] = {0x55, 0x0b, 0x21, 0x33, 0x42, 0x34, 0xCA, 0xBB}; // 手动校验
static uint8_t ZigBee_PlateTFT_test1[8] = {0x55, 0x0b, 0x20, 0x5a, 0x37, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test2[8] = {0x55, 0x0b, 0x21, 0x36, 0x4b, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test3[8] = {0x55, 0x0b, 0x20, 0x46, 0x31, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test4[8] = {0x55, 0x0b, 0x21, 0x39, 0x54, 0x33, 0x00, 0xBB};

// 交通灯
static uint8_t ZigBee_TrafficLightStartRecognition[8] = {0x55, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x01, 0xBB}; //进入识别模式

// 无线充电
static uint8_t ZigBee_WirelessChargingON[8] = {0x55, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};           //开启无线充电站

// LED显示标志物
extern uint8_t ZigBee_LEDDisplayData[8];
extern uint8_t ZigBee_LEDDisplayDistance[8];
static uint8_t ZigBee_LEDDisplayStartTimer[8] = {0x55, 0x04, 0x03, 0x01, 0x00, 0x00, 0x04, 0xBB}; // 数码管开始计时
static uint8_t ZigBee_LEDDisplayStopTimer[8] = {0x55, 0x04, 0x03, 0x00, 0x00, 0x00, 0x03, 0xBB};  // 数码管关闭计时
static uint8_t ZigBee_LEDDisplayData[8] = {0x55, 0x04, 0x02, 0xA1, 0xB2, 0xC3, 0x18, 0xBB};       // 数码管显示数据
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
static uint8_t ZigBee_AGVOpenMV[8] = {0x55, 0x02, 0x92, 0x01, 0x00, 0x00, 0x00, 0xBB};     // 启动从车二维码识别
static uint8_t ZigBee_AGVTurnLED[8] = {0x55, 0x02, 0x20, 0x01, 0x01, 0x00, 0x00, 0xBB};    // 从车转向灯
static uint8_t ZigBee_GarageLayers1[8] = {0x55, 0x0D, 0x01, 0x01, 0x00, 0x00, 0x00, 0xBB}; // 停到1层
static uint8_t ZigBee_GarageLayers2[8] = {0x55, 0x0D, 0x01, 0x02, 0x00, 0x00, 0x00, 0xBB}; // 停到2层
static uint8_t ZigBee_GarageLayers3[8] = {0x55, 0x0D, 0x01, 0x03, 0x00, 0x00, 0x00, 0xBB}; // 停到3层
static uint8_t ZigBee_GarageLayers4[8] = {0x55, 0x0D, 0x01, 0x04, 0x00, 0x00, 0x00, 0xBB}; // 停到4层

void Send_DataToUsart(uint8_t *buf, uint32_t length);
void Check_Sum(uint8_t *cmd);

#endif // __PROTOCOL_H_
