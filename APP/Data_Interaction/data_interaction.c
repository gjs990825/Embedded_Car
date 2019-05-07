#include "data_interaction.h"
#include "canP_HostCom.h"
#include "delay.h"
#include "roadway_check.h"
#include "can_user.h"
#include "tba.h"
#include "cba.h"
#include "infrared.h"
#include <string.h>
#include "Timer.h"
#include "agv.h"
#include "independent_task.h"

// 与ZigBee设备的数据交互↓

// 除从车之外其它设备的消息长度都是8位，使用ZigBee_DataStatus_t数据类型

typedef struct AGVUploadData_Struct
{
    uint8_t isSet;
    uint16_t data;
} AGVUploadData_t;

// AGV数据定义
AGVUploadData_t AGV_Ultrasonic = {.isSet = RESET, .data = 0};
AGVUploadData_t AGV_Brightness = {.isSet = RESET, .data = 0};
uint8_t AGV_QRCodeData[32];
uint8_t AGV_QRCodeLength = 0;
uint8_t AGV_QRCodeIsReceived = false;
bool AGV_MissonComplete = false;

// 标志物数据声明、定义和处理操作的结构一样所以使用宏定义简化操作

// 定义变量
#define DefineDataStatus(name) ZigBee_DataStatus_t name##_Status = {0, {0}, 0}

// 处理ZigBee返回数据
#define ProcessZigBeeReturnData(X)                \
    X##_Status.isSet = SET;                       \
    X##_Status.timeStamp = Get_GlobalTimeStamp(); \
    memcpy(X##_Status.cmd, cmd, 8)

// 标志物ZigBee数据处理
#define CaseProcess(name)                \
    case Return_##name:                  \
        ProcessZigBeeReturnData(##name); \
        break;

DefineDataStatus(BarrierGate);
DefineDataStatus(ETC);
DefineDataStatus(TrafficLight);
DefineDataStatus(StereoGarage);
DefineDataStatus(AGV);
DefineDataStatus(VoiceBroadcast);

// ZigBee指令、数据处理
void ZigBee_CmdHandler(uint8_t *cmd)
{
    switch (cmd[1])
    {
        CaseProcess(BarrierGate);
        CaseProcess(ETC);
        CaseProcess(TrafficLight);
        CaseProcess(StereoGarage);
        CaseProcess(VoiceBroadcast);

    // 从车的ZigBee数据不是八位，单独处理
    case Return_AGV:

        AGV_Status.isSet = SET;
        AGV_Status.timeStamp = Get_GlobalTimeStamp();

        switch (cmd[AGVUploadData_DataType])
        {
        case AGVUploadType_Ultrasonic: // 从车超声波
            AGV_Ultrasonic.isSet = SET;
            AGV_Ultrasonic.data = (uint16_t)(cmd[5] << 8) & 0xFF00;
            AGV_Ultrasonic.data += cmd[4] & 0x00FF;
            break;

        case AGVUploadType_Brightness: // 从车光照度
            AGV_Brightness.isSet = SET;
            AGV_Brightness.data = (uint16_t)(cmd[5] << 8) & 0xFF00;
            AGV_Brightness.data += cmd[4] & 0x00FF;
            break;

        case AGVUploadType_MisonComplete: // 从车任务完成
            AGV_MissonComplete = true;
            break;

            // 从车上传的二维码机制较为特殊
            // ID为 AGVUploadType_QRCodeData = 0x92

        case AGVUploadType_QRCodeData: // 从车二维码数据
            if (cmd[3] == 0x01)        // 记录识别成功的数据
            {
                AGV_QRCodeIsReceived = true;
                AGV_QRCodeLength = cmd[4];
                memcpy(AGV_QRCodeData, &cmd[5], AGV_QRCodeLength);
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

// 获取从车二维码是否上传 **data为传入指针的地址
// 返回-1为未接收到，其它为数据长度
// 若数据为字符串可直接输出，结束符已添加
int8_t Get_AGVQRCode(uint8_t **data)
{
    if (AGV_QRCodeIsReceived)
    {
        *data = &AGV_QRCodeData[0];
        // 方便字符串打印
        AGV_QRCodeData[AGV_QRCodeLength] = '\0';
        return AGV_QRCodeLength;
    }
    else
    {
        *data = NULL;
        return -1;
    }
}

// 获取从车超声波测量值
uint16_t Get_AGVUltrasonic(void)
{
    return (AGV_Ultrasonic.isSet == SET) ? AGV_Ultrasonic.data : 0;
}

// 获取从车光照度测量值
uint16_t Get_AGVBrightness(void)
{
    return (AGV_Brightness.isSet == SET) ? AGV_Brightness.data : 0;
}

// 获取道闸状态
// true：开启 false：关闭或超时
bool Get_BarrierGateStatus(void)
{
    Reset_ZigBeeReturnStatus(BarrierGate);
    for (uint8_t i = 0; i < 3; i++)
    {

        BarrierGate_ReturnStatus();
        WaitZigBeeFlag(BarrierGate, 300);
        if (Get_ZigBeeReturnStatus(BarrierGate))
        {
            return (Get_ZigBeeReturnData(BarrierGate)[Pack_SubCmd2] == 0x05);
        }
    }
    return false;
}

// 获取立体车库当前层数 0为错误
uint8_t Get_StereoGrageLayer(void)
{
    Reset_ZigBeeReturnStatus(StereoGarage);
    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnLayer();
        WaitZigBeeFlag(StereoGarage, 300);
        if (Get_ZigBeeReturnStatus(StereoGarage))
        {
            if (Get_ZigBeeReturnData(StereoGarage)[Pack_SubCmd1] == 0x01)
                return Get_ZigBeeReturnData(StereoGarage)[Pack_SubCmd2];
            else
                return 0;
        }
    }
    return 0;
}

// 获取立体车库前后红外状态
// [0] 前侧 [1] 后侧，0未触发 1触发（无障碍时触发）
uint8_t *Get_StereoGrageInfraredStatus(void)
{
    static uint8_t IRStatus[2];

    Reset_ZigBeeReturnStatus(StereoGarage);
    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnInfraredStatus();
        WaitZigBeeFlag(StereoGarage, 300);
        if (Get_ZigBeeReturnStatus(StereoGarage))
        {
            if (Get_ZigBeeReturnData(StereoGarage)[Pack_SubCmd1] == 0x02)
                memcpy(IRStatus, &Get_ZigBeeReturnData(StereoGarage)[Pack_SubCmd2], 2);
            else
                memset(IRStatus, 0, sizeof(IRStatus));
            break;
        }
    }
    for (uint8_t i = 0; i < 2; i++)
    {
        if (IRStatus[i] == 0x02)
            IRStatus[i] = 0;
    }
    return IRStatus;
}

///////////////////////////
// 与上位机的数据交互↓

// 向上位机请求任务
void RequestToHost_Task(uint8_t request)
{
    uint8_t requestTaskArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
    requestTaskArray[Pack_MainCmd] = request;
    Send_ToHost(requestTaskArray, 8);
}

///////////////////////
/// 向上位机发送数据 ///

// 数据发送头
uint8_t dataSendHeader[] = {0x56, 0x76, 0x00, 0x00};

void Send_QRCodeData(uint8_t *QRData, uint8_t length)
{
    dataSendHeader[Data_ID] = DataSend_QRCode;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(QRData, length);
}

void Send_RFIDData(uint8_t *RFIDData, uint8_t length)
{
    dataSendHeader[Data_ID] = DataSend_RFID;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(RFIDData, length);
}

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

// 处理上位机返回的数据
void HostData_Handler(uint8_t *buf)
{
    uint8_t requestID = buf[Data_ID];

    if (requestID > 0 && requestID <= DATA_REQUEST_NUMBER) // 确认命令是否在设定范围
    {
        // 结构体数组 DataBuffer 中取出ID对应的指针，从ID号之后开始，拷贝相应的ID字节数
        memcpy(DataBuffer[requestID].buffer, &buf[Data_ID + 1], DataBuffer[requestID].Data_Length);
        DataBuffer[requestID].isSet = SET;
    }
}

///////////////////
// 向上位机请求数据

// 数据请求头
uint8_t dataRequestHeader[3] = {0x56, 0x66, 0x00};

// 单个指令请求
void HostData_RequestSingle(uint8_t requestID)
{
    dataRequestHeader[Data_ID] = requestID;
    Send_ToHost(dataRequestHeader, 3);
}

// 多指令请求
void HostData_RequestMulti(uint8_t requestID, uint8_t *param, uint8_t paramLen)
{
    dataRequestHeader[Data_ID] = requestID;
    Send_ToHost(dataRequestHeader, 3);
    Send_ToHost(param, paramLen);
}

// 置位接收标志位
#define ResetDataIsSet(requestID) DataBuffer[requestID].isSet = RESET

// 置位后请求单指令并等待
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

// 置位后请求多指令并等待
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

// 返回ID对应的Buffer
#define ReturnBuffer(requestID) return DataBuffer[requestID].buffer

// 获取车牌号（字符串）
uint8_t *Get_PlateNumber(void)
{
    ResetAndRquest(DataRequest_PlateNumber, 300, 3);
    ReturnBuffer(DataRequest_PlateNumber);
}

// 获取二维码（字符串）
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

// 获取交通灯状态
uint8_t Get_TrafficLight(void)
{
    ResetAndRquest(DataRequest_TrafficLight, 300, 3);
    ReturnBuffer(DataRequest_TrafficLight)[0];
}

// 获取某个形状的图形个数
uint8_t Get_ShapeNumber(uint8_t Shape)
{
    uint8_t buf[1] = {Shape};
    ResetAndRquestMulti(DataRequest_ShapeNumber, buf, 1, 300, 3);
    ReturnBuffer(DataRequest_ShapeNumber)[0];
}

// 获取某个颜色的图形个数
uint8_t Get_ColorNumber(uint8_t Color)
{
    uint8_t buf[1] = {Color};
    ResetAndRquestMulti(DataRequest_ColorNumber, buf, 1, 300, 3);
    ReturnBuffer(DataRequest_ColorNumber)[0];
}

// 获取某个特定形状颜色的图形个数
uint8_t Get_ShapeColorNumber(uint8_t Shape, uint8_t Color)
{
    uint8_t buf[2] = {Shape, Color};
    ResetAndRquestMulti(DataRequest_ShapeColorNumber, buf, 2, 300, 3);
    ReturnBuffer(DataRequest_ShapeColorNumber)[0];
}

// 获取RFID处理结果
uint8_t *Get_RFIDInfo(uint8_t *data)
{
    ResetAndRquestMulti(DataRequest_RFID, data, 16, 300, 3);
    ReturnBuffer(DataRequest_RFID);
}

// 获取图形信息（字符串）
uint8_t *Get_ShapeInfo(void)
{
    ResetAndRquest(DataRequest_Preset1, 300, 3);
    ReturnBuffer(DataRequest_Preset1);
}

// 获取所有出线的颜色数量
uint8_t Get_AllColorCount(void)
{
    ResetAndRquest(DataRequest_Preset2, 300, 3);
    ReturnBuffer(DataRequest_Preset2)[0];
}
