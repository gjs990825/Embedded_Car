#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "canp_hostcom.h"

// ��������ģʽ
#define CONNECTION_MODE CONNECTION_WIFI

enum
{
    CONNECTION_SERIAL = 0,
    CONNECTION_WIFI
};

// ����Ӧѡ���ͺ���
#if CONNECTION_MODE
#define Send_ToHost Send_DataToUsart
#else
#define Send_ToHost Send_WifiData_To_Fifo
#endif // CONNECTION_MODE

// ִ��N�Σ����ӳ�
#define ExcuteNTimes(task, N, delay)    \
    do                                  \
    {                                   \
        for (uint8_t i = 0; i < N; i++) \
        {                               \
            task;                       \
            delay_ms(delay);            \
        }                               \
    } while (0)

// ����N��ZigBee���ݣ�Ĭ�ϰ�λ���ݣ�
#define Send_ZigBeeData(data, ntimes, delay)                           \
    do                                                                 \
    {                                                                  \
        ExcuteNTimes(Send_ZigbeeData_To_Fifo(data, 8), ntimes, delay); \
    } while (0)

// ������󲢷���
#define Request_ToHost(request)                     \
    do                                              \
    {                                               \
        Request_ToHostArray[Pack_Ending] = request; \
        Send_ToHost(Request_ToHostArray, 8);        \
    } while (0)

// �ϴ����ݰ��ṹ
typedef enum
{
    Pack_Header1 = 0,
    Pack_Header2,
    Pack_MainCmd,
    Pack_SubCmd1,
    Pack_SubCmd2,
    Pack_SubCmd3,
    Pack_Sum,
    Pack_Ending
} DataPackUpLoad_t;

/***************************************��������**************************************************/

// ��λ��û�н�������У�飬У���(command[Pack_Sum])����
// ��ͷΪ 0x55, 0x03 ����βΪ 0xBB ��ָ��(������λ������), Request_ToHost[Pack_Ending]�滻Ϊ������
static uint8_t Request_ToHostArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
static uint8_t Request_QRCode1 = 0x01;           // ��ά��1
static uint8_t Request_QRCode2 = 0x02;           // ��ά��2
static uint8_t Request_StreetLight = 0x31;       // ����·��
static uint8_t Request_Garage = 0x05;            // ���峵��
static uint8_t Request_TFTShow = 0x06;           // TFT��ʾ
static uint8_t Request_BarrierGate = 0x03;       // ��բ��ʾ
static uint8_t Request_LEDShow = 0x08;           // LED��־����ʾ
static uint8_t Request_PlateRecognition = 0x11;  // ����ʶ��
static uint8_t Request_ShapeRecongnition = 0x21; // ͼ��ʶ��
static uint8_t Request_AGV = 0x07;               // AGV
static uint8_t Request_Ultrasonic = 0x41;        // ������
static uint8_t Request_Alarm = 0x51;             // ����̨
static uint8_t Request_TrafficLight = 0x81;      // ��ͨ��
static uint8_t Request_RotatingLED = 0x91;       // ��תLED(������ʾ)

// ��ͷΪ 0x55, 0x0D ����βΪ 0xBB ��ָ��(�ڶ�λ��ָ��)(��ȡ��Ϣ)
static uint8_t Request_GarageFloor[2] = {0x02, 0x01}; // ���󳵿�λ�ڵڼ���
static uint8_t Request_Infrared[2] = {0x02, 0x02};    // �������

/***************************************USER************************************************/

// ��������
static uint8_t Infrared_PhotoPrevious[4] = {0x80, 0x7F, 0x05, ~(0x05)};       // ��Ƭ�Ϸ�
static uint8_t Infrared_PhotoNext[4] = {0x80, 0x7F, 0x1B, ~(0x1B)};           // ��Ƭ�·�
static uint8_t Infrared_LightAdd1[4] = {0x00, 0xFF, 0x0C, ~(0x0C)};           // ��Դ��λ��1
static uint8_t Infrared_LightAdd2[4] = {0x00, 0xFF, 0x18, ~(0x18)};           // ��Դ��λ��2
static uint8_t Infrared_LightAdd3[4] = {0x00, 0xFF, 0x5E, ~(0x5E)};           // ��Դ��λ��3
static uint8_t Infrared_TunnelFanOn[4] = {0x00, 0xFF, 0x45, ~(0x45)};         // �������ϵͳ��
static uint8_t Infrared_AlarmON[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};    // ��������
static uint8_t Infrared_AlarmOFF[6] = {0x67, 0x34, 0x78, 0xA2, 0xFD, 0x27};   // �������ر�
static uint8_t Infrared_PlateData1[6] = {0xFF, 0x20, 0x41, 0x31, 0x42, 0x32}; // ������Ϣ1
static uint8_t Infrared_PlateData2[6] = {0xFF, 0x10, 0x43, 0x35, 0x45, 0x31}; // ������Ϣ2
// ZigBee ����
static uint8_t ZigBee_BarrierGateOPEN[8] = {0x55, 0x03, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};  // ��բ����
static uint8_t ZigBee_BarrierGateCLOSE[8] = {0x55, 0x03, 0x01, 0x02, 0x00, 0x00, 0x03, 0xBB}; // ��բ�ر�
static uint8_t ZigBee_TFTPagePrevious[8] = {0x55, 0x0b, 0x10, 0x01, 0x00, 0x00, 0x11, 0xbb};  // TFT���Ϸ�ҳ
static uint8_t ZigBee_TFTPageNext[8] = {0x55, 0x0b, 0x10, 0x02, 0x00, 0x00, 0x12, 0xbb};      // TFT���·�ҳ
static uint8_t ZigBee_TFTPageAuto[8] = {0x55, 0x0b, 0x10, 0x03, 0x00, 0x00, 0x13, 0xbb};      // TFT�Զ���ҳ

// ��δʹ��
// static uint8_t CP_G1[6] = {0xFF, 0x12, 0x01, 0x00, 0x00, 0x00};
// static uint8_t CP_G2[6] = {0xFF, 0x13, 0x01, 0x00, 0x00, 0x00};

// ��ʾ����(��բ/TFT)(ZigBee Data)
static uint8_t ZigBee_PlateBarrierGate_1[8] = {0x55, 0x03, 0x10, 0x43, 0x36, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_2[8] = {0x55, 0x03, 0x11, 0x38, 0x47, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test1[8] = {0x55, 0x03, 0x10, 0x5a, 0x37, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test2[8] = {0x55, 0x03, 0x11, 0x36, 0x4b, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test3[8] = {0x55, 0x03, 0x10, 0x46, 0x31, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateBarrierGate_test4[8] = {0x55, 0x03, 0x11, 0x39, 0x54, 0x33, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_1[8] = {0x55, 0x0b, 0x20, 0x4a, 0x37, 0x36, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_2[8] = {0x55, 0x0b, 0x21, 0x31, 0x46, 0x34, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test1[8] = {0x55, 0x0b, 0x20, 0x5a, 0x37, 0x37, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test2[8] = {0x55, 0x0b, 0x21, 0x36, 0x4b, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test3[8] = {0x55, 0x0b, 0x20, 0x46, 0x31, 0x31, 0x00, 0xBB};
static uint8_t ZigBee_PlateTFT_test4[8] = {0x55, 0x0b, 0x21, 0x39, 0x54, 0x33, 0x00, 0xBB};

/***************************************Zigbee��������**************************************************/

// ��ͨ��
static uint8_t ZigBee_TrafficLightStartRecognition[8] = {0x55, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x01, 0xBB}; //����ʶ��ģʽ
static uint8_t ZigBee_WirelessChargingON[8] = {0x55, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};           //�������߳��վ
// LED��ʾ��־��
// extern uint8_t ZigBee_LEDDisplayData[8];
// extern uint8_t ZigBee_LEDDisplayDistance[8];
static uint8_t ZigBee_LEDDisplayStartTimer[8] = {0x55, 0x04, 0x03, 0x01, 0x00, 0x00, 0x04, 0xBB}; // ����ܿ�ʼ��ʱ
static uint8_t ZigBee_LEDDisplayStopTimer[8] = {0x55, 0x04, 0x03, 0x00, 0x00, 0x00, 0x03, 0xBB};  // ����ܹرռ�ʱ
static uint8_t ZigBee_LEDDisplayData[8] = {0x55, 0x04, 0x02, 0xA1, 0xB2, 0xC3, 0x18, 0xBB};       // �������ʾ����
static uint8_t ZigBee_LEDDisplayDistance[8] = {0x55, 0x04, 0x04, 0x00, 0x02, 0x00, 0x06, 0xBB};   // �������ʾ����
// ��������ָ��
static uint8_t ZigBee_VoiceRandom[8] = {0x55, 0x06, 0x20, 0x01, 0x00, 0x00, 0x00, 0xBB};         // �����������ָ��
static uint8_t ZigBee_VoiceTurnRight[8] = {0x55, 0x06, 0x10, 0x02, 0x00, 0x00, 0x12, 0xBB};      // ����ת��
static uint8_t ZigBee_VoiceNOTurnRight[8] = {0x55, 0x06, 0x10, 0x03, 0x00, 0x00, 0x13, 0xBB};    // ��ֹ��ת
static uint8_t ZigBee_VoiceDriveLeft[8] = {0x55, 0x06, 0x10, 0x04, 0x00, 0x00, 0x14, 0xBB};      // �����ʻ
static uint8_t ZigBee_VoiceNODriveLeft[8] = {0x55, 0x06, 0x10, 0x05, 0x00, 0x00, 0x15, 0xBB};    // ���б���
static uint8_t ZigBee_VoiceTurnAround[8] = {0x55, 0x06, 0x10, 0x06, 0x00, 0x00, 0x16, 0xBB};     // ԭ�ص�ͷ
static uint8_t ZigBee_VoiceDriveAssistant[8] = {0x55, 0x06, 0x10, 0x01, 0x00, 0x00, 0x11, 0xBB}; // ��ʻ����

/***************************************�ӳ�����************************************************/
extern uint8_t ZigBee_AGVStart[8];                                                         // �ӳ���������
static uint8_t ZigBee_AGVOpenMV[8] = {0x55, 0x02, 0x92, 0x01, 0x00, 0x00, 0x00, 0xBB};     // �����ӳ���ά��ʶ��
static uint8_t ZigBee_AGVTurnLED[8] = {0x55, 0x02, 0x20, 0x01, 0x01, 0x00, 0x00, 0xBB};    // �ӳ�ת���
static uint8_t ZigBee_GarageLayers1[8] = {0x55, 0x0D, 0x01, 0x01, 0x00, 0x00, 0x00, 0xBB}; // ͣ��1��
static uint8_t ZigBee_GarageLayers2[8] = {0x55, 0x0D, 0x01, 0x02, 0x00, 0x00, 0x00, 0xBB}; // ͣ��2��
static uint8_t ZigBee_GarageLayers3[8] = {0x55, 0x0D, 0x01, 0x03, 0x00, 0x00, 0x00, 0xBB}; // ͣ��3��
static uint8_t ZigBee_GarageLayers4[8] = {0x55, 0x0D, 0x01, 0x04, 0x00, 0x00, 0x00, 0xBB}; // ͣ��4��

void Send_DataToUsart(uint8_t *buf, uint32_t length);

#endif // __PROTOCOL_H_
