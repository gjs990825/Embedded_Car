#include "data_interaction.h"
#include "canP_HostCom.h"
#include "delay.h"
#include "roadway_check.h"
#include "can_user.h"
#include "tba.h"
#include "data_base.h"
#include "cba.h"
#include "infrared.h"
#include <string.h>
#include "Timer.h"
#include "agv.h"
#include "independent_task.h"

// 处理上位机发送的数据
void Process_DataFromHost(uint8_t mainCmd)
{
    switch (mainCmd)
    {
    case FromHost_Start:
        break; // 小车启动命令

    case FromHost_TFTRecognition:
        break; // TFT识别完成

    case FromHost_TrafficLight:
        break; // 交通灯识别完成

    case FromHost_QRCodeRecognition:
        break; // 二维码识别完成

    default:
        break;
    }
    SetCmdFlag(mainCmd);
}

#define DefineDataStatus(name) ZigBee_DataStatus_t name##_Status = {0, {0}, 0}
#define ProcessZigBeeReturnData(X)                \
    X##_Status.isSet = SET;                       \
    X##_Status.timeStamp = Get_GlobalTimeStamp(); \
    memcpy(X##_Status.cmd, cmd, 8)

#define CaseProcess(name)                \
    case Return_##name:                  \
        ProcessZigBeeReturnData(##name); \
        break;

DefineDataStatus(BarrierGate);
DefineDataStatus(ETC);
DefineDataStatus(AGVComplete);
DefineDataStatus(TrafficLight);
DefineDataStatus(StereoGarage);
DefineDataStatus(AGV);
DefineDataStatus(VoiceBroadcast);

// ZigBee指令处理
void ZigBee_CmdHandler(uint8_t *cmd)
{
    switch (cmd[1])
    {
        CaseProcess(BarrierGate);
        CaseProcess(ETC);
        CaseProcess(AGVComplete);
        CaseProcess(TrafficLight);
        CaseProcess(StereoGarage);
        CaseProcess(AGV);
        CaseProcess(VoiceBroadcast);
    default:
        break;
    }
}

// 处理上位机返回的数据
void HostData_Handler(uint8_t *buf)
{
    if (buf[Data_RequestID] > 0 && buf[Data_RequestID] <= DATA_REQUEST_NUMBER) // 确认命令是否在设定范围
    {
        // 结构体数组 DataBuffer 中取出ID对应的指针，从ID号之后开始，拷贝相应的ID字节数
        memcpy(DataBuffer[buf[Data_RequestID]].buffer, &buf[Data_RequestID + 1], DataBuffer[buf[Data_RequestID]].Data_Length);
        DataBuffer[buf[Data_RequestID]].isSet = SET;
    }
}

uint8_t HostDataRequestHeader[3] = {0x56, 0x66, 0x00};

void HostData_RequestSingle(uint8_t requestID)
{
    HostDataRequestHeader[Data_RequestID] = requestID;
    Send_ToHost(HostDataRequestHeader, 3);
}

void HostData_RequestMulti(uint8_t requestID, uint8_t *param, uint8_t paramLen)
{
    HostDataRequestHeader[Data_RequestID] = requestID;
    Send_ToHost(HostDataRequestHeader, 3);
    Send_ToHost(param, paramLen);
}

#define ResetDataIsSet(requestID) DataBuffer[requestID].isSet = RESET

#define ResetAndRquest(requestID, timeout, retry)                       \
    do                                                                  \
    {                                                                   \
        ResetDataIsSet(requestID);                                      \
        for (uint8_t i = 0; i < retry; i++)                             \
        {                                                               \
            HostData_RequestSingle(requestID);                          \
            WaitForFlagInMs(DataBuffer[requestID].isSet, SET, timeout); \
            if (DataBuffer[requestID].isSet == SET)                     \
            {                                                           \
                break;                                                  \
            }                                                           \
        }                                                               \
    } while (0)

#define ResetAndRquestMulti(requestID, buf, buflen, timeout, retry)     \
    do                                                                  \
    {                                                                   \
        ResetDataIsSet(requestID);                                      \
        for (uint8_t i = 0; i < retry; i++)                             \
        {                                                               \
            HostData_RequestMulti(requestID, buf, buflen);              \
            WaitForFlagInMs(DataBuffer[requestID].isSet, SET, timeout); \
            if (DataBuffer[requestID].isSet == SET)                     \
            {                                                           \
                break;                                                  \
            }                                                           \
        }                                                               \
    } while (0)

#define ReturnBuffer(requestID) return DataBuffer[requestID].buffer

uint8_t *Get_PlateNumber(void)
{
    ResetAndRquest(DataRequest_PlateNumber, 300, 3);
    ReturnBuffer(DataRequest_PlateNumber);
}

uint8_t *Get_QRCode(uint8_t QRID, uint8_t use)
{
    uint8_t buf[1] = {QRID};

    do
    {
        ResetDataIsSet(QRID);
        for (uint8_t i = 0; i < 3; i++)
        {
            HostData_RequestMulti(QRID, buf, 1);
            WaitForFlagInMs(DataBuffer[QRID].isSet, SET, 300);
            if (DataBuffer[QRID].isSet == SET)
            {
                break;
            }
        }
    } while (0);

    ReturnBuffer(QRID);
}

uint8_t Get_TrafficLight(void)
{
    ResetAndRquest(DataRequest_TrafficLight, 300, 3);
    ReturnBuffer(DataRequest_TrafficLight)[0];
}

uint8_t Get_ShapeNumber(uint8_t Shape)
{
    uint8_t buf[1] = {Shape};
    ResetAndRquestMulti(DataRequest_ShapeNumber, buf, 1, 300, 3);
    ReturnBuffer(DataRequest_ShapeNumber)[0];
}

uint8_t Get_ColorNumber(uint8_t Color)
{
    uint8_t buf[1] = {Color};
    ResetAndRquestMulti(DataRequest_ColorNumber, buf, 1, 300, 3);
    ReturnBuffer(DataRequest_ColorNumber)[0];
}

uint8_t Get_ShapeColorNumber(uint8_t Shape, uint8_t Color)
{
    uint8_t buf[2] = {Shape, Color};
    ResetAndRquestMulti(DataRequest_ShapeColorNumber, buf, 2, 300, 3);
    ReturnBuffer(DataRequest_ShapeColorNumber)[0];
}

uint8_t *Get_RFIDInfo(uint8_t *data)
{
    ResetAndRquestMulti(DataRequest_RFID, data, 16, 300, 3);
    ReturnBuffer(DataRequest_RFID);
}

uint8_t *Get_ShapeInfo(void)
{
    ResetAndRquest(DataRequest_Preset1, 300, 3);
    ReturnBuffer(DataRequest_Preset1);
}

uint8_t Get_AllColorCount(void)
{
    ResetAndRquest(DataRequest_Preset2, 300, 3);
    ReturnBuffer(DataRequest_Preset2)[0];
}
