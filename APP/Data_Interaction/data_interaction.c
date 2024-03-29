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
bool AGV_MissionComplete = false;

// 标志物数据声明、定义和处理操作的结构一样所以使用宏定义简化操作

// 定义变量
#define DefineDataStatus(name) ZigBee_DataStatus_t name##_Status = {0, {0}, 0}

// 处理ZigBee返回数据
#define ProcessZigBeeReturnData(X)   \
    X##_Status.isSet = SET;          \
    X##_Status.timeStamp = millis(); \
    memcpy(X##_Status.cmd, cmd, 8)

// 标志物ZigBee数据处理
#define CaseProcess(name)                \
    case ZigBeeID_##name:                \
        ProcessZigBeeReturnData(##name); \
        break;

DefineDataStatus(BarrierGate);
DefineDataStatus(ETC);
DefineDataStatus(TrafficLight_A);
DefineDataStatus(TrafficLight_B);
DefineDataStatus(StereoGarage_A);
DefineDataStatus(StereoGarage_B);
DefineDataStatus(AGV);
DefineDataStatus(VoiceBroadcast);

// ZigBee指令、数据处理
void ZigBee_CmdHandler(uint8_t *cmd)
{
    switch (cmd[1])
    {
        CaseProcess(BarrierGate);
        CaseProcess(ETC);
        CaseProcess(TrafficLight_A);
        CaseProcess(TrafficLight_B);
        CaseProcess(StereoGarage_A);
        CaseProcess(StereoGarage_B);
        CaseProcess(VoiceBroadcast);

    // 从车的ZigBee数据不是八位，单独处理
    case ZigBeeID_AGV:

        AGV_Status.isSet = SET;
        AGV_Status.timeStamp = millis();

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
            AGV_MissionComplete = true;
            break;

            // 从车上传的二维码机制较为特殊
            // ID为 AGVUploadType_QRCodeData = 0x92

        case AGVUploadType_QRCodeData: // 从车二维码数据
            if (cmd[3] == 0x01)        // 记录识别成功的数据
            {
                AGV_QRCodeIsReceived = true;
                AGV_QRCodeLength = cmd[4];
                memcpy(AGV_QRCodeData, &cmd[5], AGV_QRCodeLength);
                Send_QRCodeData(AGV_QRCodeData, AGV_QRCodeLength);
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
uint8_t Get_StereoGrageLayer(uint8_t garage_x)
{
    ZigBee_DataStatus_t *StereoGarage_Status;
    StereoGarage_Status = (garage_x == StereoGarage_A) ? (&StereoGarage_A_Status) : (&StereoGarage_B_Status);

    StereoGarage_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnLayer(garage_x);
        WaitForFlagInMs(StereoGarage_Status->isSet, SET, 300);

        if (StereoGarage_Status->isSet == SET)
        {
            if (StereoGarage_Status->cmd[Pack_SubCmd1] == 0x01)
                return StereoGarage_Status->cmd[Pack_SubCmd2];
            else
                return 0;
        }
    }
    return 0;
}

// 获取立体车库前后红外状态
// [0] 前侧 [1] 后侧，0未触发 1触发（无障碍时触发）
uint8_t *Get_StereoGrageInfraredStatus(uint8_t garage_x)
{
    static uint8_t IRStatus[2];

    ZigBee_DataStatus_t *StereoGarage_Status;
    StereoGarage_Status = (garage_x == StereoGarage_A) ? (&StereoGarage_A_Status) : (&StereoGarage_B_Status);

    StereoGarage_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnInfraredStatus(garage_x);
        WaitForFlagInMs(StereoGarage_Status->isSet, SET, 300);

        if (StereoGarage_Status->isSet == SET)
        {
            if (StereoGarage_Status->cmd[Pack_SubCmd1] == 0x02)
                memcpy(IRStatus, &StereoGarage_Status->cmd[Pack_SubCmd2], 2);
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
uint8_t dataSendHeader[6] = {0x56, 0x76, 0x00, 0x00};

void Send_QRCodeData(uint8_t *QRData, uint8_t length)
{
    dataSendHeader[Data_ID] = DataSend_QRCode;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(QRData, length);
}

// 这个跟其它不同，多了个位区分RFIDx
// 发送RFID数据
void Send_RFIDData(uint8_t RFIDx, uint8_t *RFIDData, uint8_t length)
{
    if (RFIDx > 3 || RFIDx < 1)
        return;

    dataSendHeader[Data_ID] = RFIDx + DataSend_RFID - 1;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(RFIDData, length);
}

// 发送预设接口数据
void Send_PresetData(uint8_t preset_x, uint8_t *data, uint8_t length)
{
    dataSendHeader[Data_ID] = preset_x + DataSend_Preset1 - 1;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(data, length);
}

// 处理上位机的指令，与官方主指令对应
// 只置位因为没有数据，当作标志位使用
void Process_CommandFromHost(uint8_t mainCmd)
{
    SetCmdFlag(mainCmd);
}

// 处理上位机返回的数据
// 自定义的数据接收，长度在DataBuffer结构体中定义
void HostData_Handler(uint8_t *buf)
{
    uint8_t requestID = buf[Data_ID];

    if (requestID > 0 && requestID <= DATA_REQUEST_NUMBER) // 确认命令是否在设定范围
    {
        // 结构体数组 DataBuffer 中取出ID对应的指针，从ID号之后开始，拷贝相应的ID字节数
        memcpy(DataBuffer[requestID].buffer, &buf[Data_Length], DataBuffer[requestID].Data_Length);
        // 标记已接收
        DataBuffer[requestID].isSet = SET;
    }
}

///////////////////
// 向上位机请求数据

// 返回ID对应的Buffer
#define ReturnBuffer(requestID) return DataBuffer[requestID].buffer
// 自动填充buf长度（buf不能为指针！）
#define ResetRquestWait(requestID, buf) Reset_Rquest_Wait(requestID, buf, sizeof(buf))

// 请求数据
void HostData_Request(uint8_t requestID, uint8_t *param, uint8_t paramLen)
{
    // 数据请求头
    static uint8_t dataRequestHeader[3] = {0x56, 0x66, 0x00};

    dataRequestHeader[Data_ID] = requestID;
    Send_ToHost(dataRequestHeader, 3);
    Send_ToHost(param, paramLen);
}

// 置位后请求并等待
void Reset_Rquest_Wait(uint8_t requestID, uint8_t *buf, uint8_t buflen)
{
    // 清空标志位
    DataBuffer[requestID].isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        // 发送请求
        HostData_Request(requestID, buf, buflen);
        WaitForFlagInMs(DataBuffer[requestID].isSet, SET, 300);

        // 判断返回状态
        if (DataBuffer[requestID].isSet == SET)
            break;
    }
}

// 预设的请求数据接口↓

// 获取车牌号（字符串）
uint8_t *Get_PlateNumber(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_PlateNumber, buf);
    ReturnBuffer(DataRequest_PlateNumber);
}

// 获取二维码（字符串）
uint8_t *Get_QRCode(uint8_t QRCode_x, uint8_t use)
{
    uint8_t QRID = (QRCode_x == QRCode_1) ? DataRequest_QRCode1 : DataRequest_QRCode2;
    uint8_t buf[] = {use};
    ResetRquestWait(QRID, buf);
    ReturnBuffer(QRID);
}

// 获取交通灯状态
uint8_t Get_TrafficLight(uint8_t light_x)
{
    uint8_t buf[] = {light_x};
    ResetRquestWait(DataRequest_TrafficLight, buf);
    ReturnBuffer(DataRequest_TrafficLight)[0];
}

// 获取某个形状的图形个数
uint8_t Get_ShapeNumber(uint8_t TFTx, uint8_t Shape)
{
    uint8_t buf[] = {TFTx, Shape};
    ResetRquestWait(DataRequest_ShapeNumber, buf);
    ReturnBuffer(DataRequest_ShapeNumber)[0];
}

// 获取某个颜色的图形个数
uint8_t Get_ColorNumber(uint8_t TFTx, uint8_t Color)
{
    uint8_t buf[] = {TFTx, Color};
    ResetRquestWait(DataRequest_ColorNumber, buf);
    ReturnBuffer(DataRequest_ColorNumber)[0];
}

// 获取某个特定形状颜色的图形个数
uint8_t Get_ShapeColorNumber(uint8_t TFTx, uint8_t Shape, uint8_t Color)
{
    uint8_t buf[] = {TFTx, Shape, Color};
    ResetRquestWait(DataRequest_ShapeColorNumber, buf);
    ReturnBuffer(DataRequest_ShapeColorNumber)[0];
}

// 获取RFID处理结果
uint8_t *Get_RFIDInfo(uint8_t RFIDx)
{
    uint8_t buf[] = {RFIDx};
    ResetRquestWait(DataRequest_RFID, buf);
    ReturnBuffer(DataRequest_RFID);
}

// 获取TFT信息（HEX，3bytes）
uint8_t *Get_TFTInfo(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_TFTInfo, buf);
    ReturnBuffer(DataRequest_TFTInfo);
}

// 获取所有出现的颜色数量
uint8_t Get_AllColorCount(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_AllColorCount, buf);
    ReturnBuffer(DataRequest_AllColorCount)[0];
}

// 获取所有出现的形状数量
uint8_t Get_AllShapeCount(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_AllShapeCount, buf);
    ReturnBuffer(DataRequest_AllShapeCount)[0];
}

// 获取预设接口的数据
uint8_t *Get_PresetData(uint8_t preset_x)
{
    preset_x += DataRequest_Preset1 - 1;
    Reset_Rquest_Wait(preset_x, NULL, 0);
    ReturnBuffer(preset_x);
}
