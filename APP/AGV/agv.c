#include "agv.h"
#include "protocol.h"
#include "string.h"
#include "route.h"
#include "a_star.h"

// 向AGV发送的数据buffer
uint8_t DataToAGV[] = {0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

#define ClearAGVCmd() memset(&DataToAGV[Pack_MainCmd], 0, 4)

// #define ClearAGVCmd()                \
//     do                               \
//     {                                \
//         DataToAGV[Pack_MainCmd] = 0; \
//         DataToAGV[Pack_SubCmd1] = 0; \
//         DataToAGV[Pack_SubCmd2] = 0; \
//         DataToAGV[Pack_SubCmd3] = 0; \
//     } while (0)

#define SendAGVCmd() Send_ZigBeeData(DataToAGV)

// // 基本运动控制
// void Stop(void);
// void Go_Ahead(int speed, uint16_t encoder);
// void Back_Off(int speed, uint16_t encoder);
// void Turn_ByEncoder(int16_t digree);
// void Track_ByEncoder(int speed, uint16_t setencoder);
// void Start_Tracking(int speed);

// 停止
void AGV_Stop(void)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Stop;
    SendAGVCmd();
}

// 前进
void AGV_Go(uint8_t speed, uint16_t encoder)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Go;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = encoder & 0x0F;
    DataToAGV[Pack_SubCmd3] = encoder >> 8;
    SendAGVCmd();
}

// 后退
void AGV_Back(uint8_t speed, uint16_t encoder)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Back;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = encoder & 0x0F;
    DataToAGV[Pack_SubCmd3] = encoder >> 8;
    SendAGVCmd();
}

// 转向
void AGV_Turn(int16_t speed)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = (speed > 0) ? FromHost_TurnRight : FromHost_TurnLeft;
    DataToAGV[Pack_SubCmd1] = (uint8_t)((speed > 0) ? speed : -speed);
    SendAGVCmd();
}

// 循迹
void AGV_TrackLine(uint8_t speed)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TrackLine;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

// 清空码盘值
void AGV_ClearEncoder(void)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_EncoderClear;
    SendAGVCmd();
}

// 发送红外数据(此处已移除立即发送的功能，从车自行发送)
void AGV_SendInfraredData(uint8_t irData[6])
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredFrontData;
    memcpy(&DataToAGV[Pack_SubCmd1], &irData[0], 3);
    SendAGVCmd();
    delay_ms(500);
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredBackData;
    memcpy(&DataToAGV[Pack_SubCmd1], &irData[3], 3);
    SendAGVCmd();
    // delay_ms(50);
    // ClearAGVCmd();
    // DataToAGV[Pack_MainCmd] = FromHost_InfraredSend;
    // SendAGVCmd();
}

// 转向灯
void AGV_TurnningLightControl(bool left, bool right)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TurnningLightControl;
    DataToAGV[Pack_SubCmd1] = left ? 0x01 : 0x00;
    DataToAGV[Pack_SubCmd2] = right ? 0x01 : 0x00;
    SendAGVCmd();
}

// 蜂鸣器
void AGV_Beep(bool status)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Beep;
    DataToAGV[Pack_SubCmd1] = status ? 0x01 : 0x00;
    SendAGVCmd();
}

// 电子相册 1 下翻 0 上翻
void AGV_PhotoChange(bool dir)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = dir ? FromHost_InfraredPhotoNext : FromHost_InfraredPhotoPrevious;
    SendAGVCmd();
}

// 光源档位加（1 - 3）
void AGV_LightAdd(uint8_t level)
{
    if (level > 3 || level < 1)
    {
        return;
    }
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = level + 0x60;
    SendAGVCmd();
}

// 返回数据
void AGV_UploadData(bool sta)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVReturnData;
    DataToAGV[Pack_SubCmd1] = sta;
    SendAGVCmd();
}

// 语音识别
void AGV_VoiceRecognition(bool sta)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_VoiceRecognition;
    DataToAGV[Pack_SubCmd1] = sta;
    SendAGVCmd();
}

// AGV启动
void AGV_Start(void)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVStart;
    SendAGVCmd();
}

void AGV_SetTowards(uint8_t towards)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_ReceivePresetHeadTowards;
    DataToAGV[Pack_SubCmd1] = towards;
    SendAGVCmd();
}

void AGV_SendSinglePoint(uint8_t number, RouteNode_t node)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVRouting;
    DataToAGV[Pack_SubCmd1] = number;
    DataToAGV[Pack_SubCmd2] = node.x;
    DataToAGV[Pack_SubCmd3] = node.y;
    SendAGVCmd();
}

void AGV_SetRouteFromTask(RouteNode_t task[], uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        AGV_SendSinglePoint(i, task[i]);
        delay_ms(700);
        AGV_SendSinglePoint(i, task[i]);
        delay_ms(700);
    }
}

// 发送规划的路径到从车
void AGV_SetRoute(uint8_t *str)
{
    uint8_t length = strlen((char *)str) / 2;
    RouteNode_t tempNode;

    for (uint8_t i = 0; i < length; i++)
    {
        tempNode = Coordinate_Covent(&str[i * 2]);
        AGV_SendSinglePoint(i, tempNode);
        delay_ms(700);
        AGV_SendSinglePoint(i, tempNode);
        delay_ms(700);
    }
}

// 设定从车的任务
void AGV_SetTaskID(uint8_t routeNumber, uint8_t taskNumber)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVSetTask;
    DataToAGV[Pack_SubCmd1] = routeNumber;
    DataToAGV[Pack_SubCmd2] = taskNumber;
    SendAGVCmd();
    delay_ms(700);
    SendAGVCmd();
    delay_ms(700);
}
