#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "protocol.h"

// 自动判断数据长度 _A(Auto)
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

// 快速发送单个指令(次数和延时在宏传递的参数中定义)
#define LED_TimerStart() Send_ZigBeeData(ZigBee_LEDDisplayStartTimer, 3, 20) // 开始计时
#define LED_TimerStop() Send_ZigBeeData(ZigBee_LEDDisplayStopTimer, 3, 20)   // 停止计时
#define TFTPage_Next() Send_ZigBeeData(ZigBee_TFTPageNext, 2, 100)           // TFT下一页

// 请求指令
#define Request_QRCode1() Request_ToHost(RequestCmd_QRCode1)                     // 二维码1
#define Request_QRCode2() Request_ToHost(RequestCmd_QRCode2)                     // 二维码2
#define Request_StreetLight() Request_ToHost(RequestCmd_StreetLight)             // 智能路灯
#define Request_Garage() Request_ToHost(RequestCmd_Garage)                       // 立体车库
#define Request_TFTShow() Request_ToHost(RequestCmd_TFTShow)                     // TFT显示
#define Request_BarrierGate() Request_ToHost(RequestCmd_BarrierGate)             // 道闸显示
#define Request_LEDShow() Request_ToHost(RequestCmd_LEDShow)                     // LED标志物显示
#define Request_PlateRecognition() Request_ToHost(RequestCmd_PlateRecognition)   // 车牌识别
#define Request_ShapeRecongnition() Request_ToHost(RequestCmd_ShapeRecongnition) // 图形识别
#define Request_AGV() Request_ToHost(RequestCmd_AGV)                             // AGV
#define Request_Ultrasonic() Request_ToHost(RequestCmd_Ultrasonic)               // 超声波
#define Request_Alarm() Request_ToHost(RequestCmd_Alarm)                         // 报警台
#define Request_TrafficLight() Request_ToHost(RequestCmd_TrafficLight)           // 交通灯
#define Request_RotatingLED() Request_ToHost(RequestCmd_RotatingLED)             // 旋转LED(立体显示)

// 请求数据(或行为)
#define Request_GarageFloor() Request_Data(RequestData_GarageFloor) // 请求车库层数
#define Request_Infrared() Request_Data(RequestData_Infrared)       // 请求红外数据

// 红外指令

#define Send_PlateToTFT()                       \
    Send_ZigBeeData(ZigBee_PlateTFT_1, 1, 600); \
    Send_ZigBeeData(ZigBee_PlateTFT_2, 1, 600)

void TFT_Task(void);
void QRCode_Task(void);
void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);




// 任务点
void Task_5_5(void);
void Task_1_5(void);
void Task_3_5(void);
void Task_1_3(void);
void Task_5_3(void);
void Task_5_1(void);
void Task_3_1(void);

#endif // __TASK_H_
