#include "data_from_host.h"
#include "canP_HostCom.h"
#include "delay.h"
#include "data_channel.h"
#include "roadway_check.h"
#include "can_user.h"
#include "tba.h"
#include "data_base.h"
#include "cba.h"
#include "infrared.h"
#include <string.h>
#include "Timer.h"
#include "agv.h"

#define SetCmdFlag(flag) CommandFlagStatus[flag] = SET
#define SetSpeed Wifi_Rx_Buf[Pack_SubCmd1]

void SaveDataFromHost(uint8_t buf[6], uint8_t commandID, bool isFrontData)
{
    if (isFrontData)
    {
        buf[0] = Wifi_Rx_Buf[Pack_SubCmd1];
        buf[1] = Wifi_Rx_Buf[Pack_SubCmd2];
        buf[2] = Wifi_Rx_Buf[Pack_SubCmd3];
    }
    else
    {
        buf[3] = Wifi_Rx_Buf[Pack_SubCmd1];
        buf[4] = Wifi_Rx_Buf[Pack_SubCmd2];
        buf[5] = Wifi_Rx_Buf[Pack_SubCmd3];
    }
}

uint16_t GetEncoderSetting(void)
{
    uint16_t tmp = 0;
    tmp = Wifi_Rx_Buf[5];
    tmp <<= 8;
    tmp |= Wifi_Rx_Buf[4];
    return tmp;
}

void SaveToZigBee(uint8_t buf[8])
{
    buf[3] = Wifi_Rx_Buf[3];
    buf[4] = Wifi_Rx_Buf[4];
    buf[5] = Wifi_Rx_Buf[5];
}

// ������λ�����͵�����
void Process_DataFromHost(uint8_t mainCmd)
{
    switch (mainCmd)
    {
    case FromHost_Stop:
        Control(0, 0);
        Roadway_Flag_clean();
        break; // ֹͣ

    case FromHost_Go:
        Go_Ahead(SetSpeed, GetEncoderSetting());
        break; // ǰ��

    case FromHost_Back:
        Back_Off(SetSpeed, GetEncoderSetting());
        break; // ����

    case FromHost_TurnLeft:
        Turn_ByEncoder(-90);
        break; // ��ת

    case FromHost_TurnRight:
        Turn_ByEncoder(90);
        break; // ��ת

    case FromHost_TrackLine:
        Start_Tracking(SetSpeed);
        break; // ѭ��

    case FromHost_EncoderClear:

        break; // ��������

    case FromHost_TurnCountClockWiseToDigree:
        Turn_ByEncoder(-(int16_t)GetEncoderSetting());
        Control(-SetSpeed, SetSpeed);
        break; // ��ת��--�Ƕ�

    case FromHost_TurnClockWiseToDigree:
        Turn_ByEncoder((int16_t)GetEncoderSetting());
        Control(SetSpeed, -SetSpeed);
        break; // ��ת��--�Ƕ�

    case FromHost_InfraredFrontData:
        SaveDataFromHost(Infrared_Data, mainCmd, true);
        break; // ����ǰ��λ����

    case FromHost_InfraredBackData:
        SaveDataFromHost(Infrared_Data, mainCmd, false);
        break; // �������λ����

    case FromHost_InfraredSend:
        Infrared_Send_A(Infrared_Data);
        break; // ֪ͨС����Ƭ�����ͺ�����

    case FromHost_TurnningLightControl:
        Set_tba_WheelLED(L_LED, Wifi_Rx_Buf[Pack_SubCmd1]);
        Set_tba_WheelLED(R_LED, Wifi_Rx_Buf[Pack_SubCmd2]);
        break; // ת��ƿ���

    case FromHost_Beep:
        Set_tba_Beep(Wifi_Rx_Buf[Pack_SubCmd1]);
        break; // ������

    case FromHost_NotUsed:
        break; // ��δʹ��

    case FromHost_InfraredPhotoPrevious:
        Infrared_Send_A(Infrared_PhotoNext);
        break; // ���ⷢ�������Ƭ�Ϸ�

    case FromHost_InfraredPhotoNext:
        Infrared_Send_A(Infrared_PhotoPrevious);
        break; // ���ⷢ�������Ƭ�·�

    case FromHost_InfraredLightAdd1:
        Infrared_Send_A(Infrared_LightAdd1);
        break; // ���ⷢ����ƹ�Դǿ�ȵ�λ��1

    case FromHost_InfraredLightAdd2:
        Infrared_Send_A(Infrared_LightAdd2);
        break; // ���ⷢ����ƹ�Դǿ�ȵ�λ��2

    case FromHost_InfraredLightAdd3:
        Infrared_Send_A(Infrared_LightAdd3);
        break; // ���ⷢ����ƹ�Դǿ�ȵ�λ��3

    case FromHost_AGVReturnData:
        Host_AGV_Return_Flag = Wifi_Rx_Buf[Pack_SubCmd1];
        break; // �ӳ�����

    case FromHost_LEDDisplaySecomdRow:
        SaveToZigBee(ZigBee_LEDDisplayData);
        Send_ZigbeeData_To_Fifo(ZigBee_LEDDisplayData, 8);
        break; // ����ܵڶ�����ʾ������

    case FromHost_ReceivePresetHeadTowards:
        SaveToZigBee(ZigBee_AGVPreset);
        Send_ZigbeeData_To_Fifo(ZigBee_AGVPreset, 8);
        break; // ����Ԥ����ͷ����

    case FromHost_Start:
        break; // С����������

    case FromHost_QRCodeRecognition:
        Set_tba_WheelLED(L_LED, 1);
        Set_tba_WheelLED(R_LED, 1);
        break; // ��ά��ʶ��

    case FromHost_PlateRecognition:
        break; // ����ʶ��

    case FromHost_ShapeRecongnition:
        break; // ͼ��ʶ��

    case FromHost_TrafficLight:
        break; // ��ͨ��

    case FromHost_StreetLight:
        break; // ·��

    case FromHost_PlateData1:
        Infrared_PlateData1[2] = Wifi_Rx_Buf[3];
        Infrared_PlateData1[3] = Wifi_Rx_Buf[4];
        Infrared_PlateData1[4] = Wifi_Rx_Buf[5];
        break; // ������Ϣ1

    case FromHost_PlateData2:
        Infrared_PlateData1[5] = Wifi_Rx_Buf[3];
        Infrared_PlateData2[2] = Wifi_Rx_Buf[4];
        Infrared_PlateData2[3] = Wifi_Rx_Buf[5];
        break; // ������Ϣ2

    case FromHost_AlarmON:
        Infrared_AlarmData[0] = Wifi_Rx_Buf[3];
        Infrared_AlarmData[1] = Wifi_Rx_Buf[4];
        Infrared_AlarmData[2] = Wifi_Rx_Buf[5];
        break; // ��������

    case FromHost_AlarmOFF:
        Infrared_AlarmData[3] = Wifi_Rx_Buf[3];
        Infrared_AlarmData[4] = Wifi_Rx_Buf[4];
        Infrared_AlarmData[5] = Wifi_Rx_Buf[5];
        break; // ��������

    case FromHost_Garage:
        Set_tba_WheelLED(L_LED, 1);
        Set_tba_WheelLED(R_LED, 1);
        break; // ���峵��

    case FromHost_TFTRecognition:
        break; // TFTʶ��

    case FromHost_AGVStart:
        AGV_Start();
        break; // AGV����
    default:
        break;
    }
    SetCmdFlag(mainCmd);
}

ZigBee_DataStatus_t ETC_Status = {0, 0};
ZigBee_DataStatus_t BarrierGate_Status = {0, 0};
ZigBee_DataStatus_t AGVComplete_Status = {0, 0};

#define SetAndAddStamp(X) \
    X.isSet = SET;        \
    X.timeStamp = Get_GlobalTimeStamp()

// ZigBeeָ���
void ZigBee_CmdHandler(uint8_t cmd)
{
    switch (cmd)
    {
    case Return_ETC:
        SetAndAddStamp(ETC_Status);
        break;
    case Return_BarrierGate:
        SetAndAddStamp(BarrierGate_Status);
    case Return_AGVComplete:
        SetAndAddStamp(AGVComplete_Status);
    default:
        break;
    }
}

// ������λ�����ص�����
void HostData_Handler(uint8_t *buf)
{
    if (buf[Data_RequestID] > 0 && buf[Data_RequestID] <= DATA_REQUEST_NUMBER) // ȷ�������Ƿ����趨��Χ
    {
        // �ṹ������ DataBuffer ��ȡ��ID��Ӧ��ָ�룬��ID��֮��ʼ��������Ӧ��ID�ֽ���
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
    uint8_t buf[2] = {QRID, use};
    ResetAndRquestMulti(DataRequest_ShapeNumber, buf, 2, 300, 3);
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

