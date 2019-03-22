#include "agv.h"
#include "protocol.h"
#include "string.h"
#include "route.h"
#include "a_star.h"

// ��AGV���͵�����buffer
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

// // �����˶�����
// void Stop(void);
// void Go_Ahead(int speed, uint16_t encoder);
// void Back_Off(int speed, uint16_t encoder);
// void Turn_ByEncoder(int16_t digree);
// void Track_ByEncoder(int speed, uint16_t setencoder);
// void Start_Tracking(int speed);

// ֹͣ
void AGV_Stop(void)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Stop;
    SendAGVCmd();
}

// ǰ��
void AGV_Go(uint8_t speed, uint16_t encoder)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Go;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = encoder & 0x0F;
    DataToAGV[Pack_SubCmd3] = encoder >> 8;
    SendAGVCmd();
}

// ����
void AGV_Back(uint8_t speed, uint16_t encoder)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Back;
    DataToAGV[Pack_SubCmd1] = speed;
    DataToAGV[Pack_SubCmd2] = encoder & 0x0F;
    DataToAGV[Pack_SubCmd3] = encoder >> 8;
    SendAGVCmd();
}

// ת��
void AGV_Turn(int16_t speed)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = (speed > 0) ? FromHost_TurnRight : FromHost_TurnLeft;
    DataToAGV[Pack_SubCmd1] = (uint8_t)((speed > 0) ? speed : -speed);
    SendAGVCmd();
}

// ѭ��
void AGV_TrackLine(uint8_t speed)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TrackLine;
    DataToAGV[Pack_SubCmd1] = speed;
    SendAGVCmd();
}

// �������ֵ
void AGV_ClearEncoder(void)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_EncoderClear;
    SendAGVCmd();
}

// ���ͺ�������
void AGV_SendInfraredData(uint8_t irData[6])
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredFrontData;
    DataToAGV[Pack_SubCmd1] = irData[0];
    DataToAGV[Pack_SubCmd2] = irData[1];
    DataToAGV[Pack_SubCmd3] = irData[2];
    SendAGVCmd();
    delay_ms(50);
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredBackData;
    DataToAGV[Pack_SubCmd1] = irData[3];
    DataToAGV[Pack_SubCmd2] = irData[4];
    DataToAGV[Pack_SubCmd3] = irData[5];
    SendAGVCmd();
    delay_ms(50);
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_InfraredSend;
    SendAGVCmd();
}

// ת���
void AGV_TurnningLightControl(bool left, bool right)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_TurnningLightControl;
    DataToAGV[Pack_SubCmd1] = left ? 0x01 : 0x00;
    DataToAGV[Pack_SubCmd2] = right ? 0x01 : 0x00;
    SendAGVCmd();
}

// ������
void AGV_Beep(bool status)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_Beep;
    DataToAGV[Pack_SubCmd1] = status ? 0x01 : 0x00;
    SendAGVCmd();
}

// ������� 1 �·� 0 �Ϸ�
void AGV_PhotoChange(bool dir)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = dir ? FromHost_InfraredPhotoNext : FromHost_InfraredPhotoPrevious;
    SendAGVCmd();
}

// ��Դ��λ�ӣ�1 - 3��
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

// ��������
void AGV_UploadData(bool sta)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVReturnData;
    DataToAGV[Pack_SubCmd1] = sta;
    SendAGVCmd();
}

// ����ʶ��
void AGV_VoiceRecognition(bool sta)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_VoiceRecognition;
    DataToAGV[Pack_SubCmd1] = sta;
    SendAGVCmd();
}

// AGV����
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

void AGV_SendSingle(uint8_t number, RouteNode node)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = FromHost_AGVRouting;
    DataToAGV[Pack_SubCmd1] = number;
    DataToAGV[Pack_SubCmd2] = node.x;
    DataToAGV[Pack_SubCmd3] = node.y;
    SendAGVCmd();
}

void AGV_SetRouteFromTask(RouteNode task[], uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        AGV_SendSingle(i, task[i]);
        delay_ms(700);
        AGV_SendSingle(i, task[i]);
        delay_ms(700);
    }
}

// ת���ַ����������
RouteNode Coordinate_Covent(uint8_t str[2])
{
    RouteNode outNode;
    outNode.dir = DIR_NOTSET;

    switch (str[0])
    {
    case 'A':
        outNode.x = 0;
        break;
    case 'B':
        outNode.x = 1;
        break;
    case 'C':
        outNode.x = 2;
        break;
    case 'D':
        outNode.x = 3;
        break;
    case 'E':
        outNode.x = 4;
        break;
    case 'F':
        outNode.x = 5;
        break;
    case 'G':
        outNode.x = 6;
        break;

    default:
        outNode.x = -1;
        break;
    }
    switch (str[1])
    {
    case '1':
        outNode.y = 6;
        break;
    case '2':
        outNode.y = 5;
        break;
    case '3':
        outNode.y = 4;
        break;
    case '4':
        outNode.y = 3;
        break;
    case '5':
        outNode.y = 2;
        break;
    case '6':
        outNode.y = 1;
        break;
    case '7':
        outNode.y = 0;
        break;
    default:
        outNode.y = -1;
        break;
    }

    return outNode;
}

// ת������㵽�ַ���
uint8_t *ReCoordinate_Covent(uint8_t x, uint8_t y)
{
    static uint8_t tempCoord[2];

    switch (x)
    {
    case 0:
        tempCoord[0] = 'A';
        break;
    case 1:
        tempCoord[0] = 'B';
        break;
    case 2:
        tempCoord[0] = 'C';
        break;
    case 3:
        tempCoord[0] = 'D';
        break;
    case 4:
        tempCoord[0] = 'E';
        break;
    case 5:
        tempCoord[0] = 'F';
        break;
    case 6:
        tempCoord[0] = 'G';
        break;
    default:
        tempCoord[0] = '\0';
        break;
    }

    switch (y)
    {
    case 0:
        tempCoord[1] = '7';
        break;
    case 1:
        tempCoord[1] = '6';
        break;
    case 2:
        tempCoord[1] = '5';
        break;
    case 3:
        tempCoord[1] = '4';
        break;
    case 4:
        tempCoord[1] = '3';
        break;
    case 5:
        tempCoord[1] = '2';
        break;
    case 6:
        tempCoord[1] = '1';
        break;

    default:
        tempCoord[1] = '\0';
        break;
    }

    return tempCoord;
}

// ���͹滮��·�����ӳ�
void AGV_SetRoute(uint8_t *str)
{
    uint8_t length = strlen((char *)str) / 2;
    RouteNode tempNode;

    for (uint8_t i = 0; i < length; i++)
    {
        tempNode = Coordinate_Covent(&str[i * 2]);
        AGV_SendSingle(i, tempNode);
        delay_ms(700);
        AGV_SendSingle(i, tempNode);
        delay_ms(700);
    }
}
