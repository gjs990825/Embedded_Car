#include "agv.h"
#include "protocol.h"

// 向AGV发送的数据buffer
uint8_t DataToAGV[] = {0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

#define ClearAVGCmd()                \
    do                               \
    {                                \
        DataToAGV[Pack_MainCmd] = 0; \
        DataToAGV[Pack_SubCmd1] = 0; \
        DataToAGV[Pack_SubCmd2] = 0; \
        DataToAGV[Pack_SubCmd3] = 0; \
    } while (0)

#define SendAGVCmd() AGV_SendCmd(DataToAGV)

void AGV_SendCmd(uint8_t *cmd)
{
    Check_Sum(cmd);
    Send_ZigbeeData_To_Fifo(cmd, 8);
}

// // 基本运动控制
// void Stop(void);
// void Go_Ahead(int speed, uint16_t mp);
// void Back_Off(int speed, uint16_t mp);
// void Turn_ByEncoder(int16_t digree);
// void Track_ByEncoder(int speed, uint16_t setMP);
// void Start_Tracking(int speed);

void AGV_Stop(void)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Stop;
    SendAGVCmd();
}

void AGV_Go(uint8_t speed, uint16_t mp)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Go;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = mp & 0x0F;
    DataToAGV[Pack_SubCmd3] = mp >> 8;
    SendAGVCmd();
}

void AGV_Back(uint8_t speed, uint16_t mp)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Back;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = mp & 0x0F;
    DataToAGV[Pack_SubCmd3] = mp >> 8;
    SendAGVCmd();
}

void AGV_TurnLeft(uint8_t speed)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TurnLeft;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

void AGV_TurnRight(uint8_t speed)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TurnRight;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

void AGV_TrackLine(uint8_t speed)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TrackLine;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

void AGV_ClearEncoder(void)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_EncoderClear;
    SendAGVCmd();
}

void AGV_TrackLine(uint8_t speed)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TrackLine;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

void AGV_TurnByEncoder(uint8_t speed, uint16_t encoder)
{
    ClearAVGCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TrackLine;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}


// {
//     ClearAVGCmd();

//     SendAGVCmd();
// }



