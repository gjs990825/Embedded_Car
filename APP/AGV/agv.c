#include "agv.h"
#include "protocol.h"
#include "string.h"

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

#define SendAGVCmd() AGV_SendCmd(DataToAGV)

void AGV_SendCmd(uint8_t *cmd)
{
    Check_Sum(cmd);
    Send_ZigbeeData_To_Fifo(cmd, 8);
}

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

// 发送红外数据
void AGV_SendInfraredData(uint8_t irData[6])
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredFrontData;
    DataToAGV[Pack_SubCmd1] = irData[0];
    DataToAGV[Pack_SubCmd2] = irData[1];
    DataToAGV[Pack_SubCmd3] = irData[2];
    SendAGVCmd();
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredBackData;
    DataToAGV[Pack_SubCmd1] = irData[3];
    DataToAGV[Pack_SubCmd2] = irData[4];
    DataToAGV[Pack_SubCmd3] = irData[5];
    SendAGVCmd();
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredSend;
    SendAGVCmd();
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
    DataToAGV[Pack_MainCmd] = FromHost_AGVReturn;
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

