#if !defined(__TASK_H_)
#define __TASK_H_

#include "sys.h"
#include "protocol.h"

// �Զ��ж����ݳ��� _A(Auto)
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

// ���ٷ��͵���ָ��(��������ʱ�ں괫�ݵĲ����ж���)
#define LED_TimerStart() Send_ZigBeeData(ZigBee_LEDDisplayStartTimer, 3, 20) // ��ʼ��ʱ
#define LED_TimerStop() Send_ZigBeeData(ZigBee_LEDDisplayStopTimer, 3, 20)   // ֹͣ��ʱ
#define TFTPage_Next() Send_ZigBeeData(ZigBee_TFTPageNext, 2, 100)           // TFT��һҳ

// ����ָ��
#define Request_QRCode1() Request_ToHost(RequestCmd_QRCode1)                     // ��ά��1
#define Request_QRCode2() Request_ToHost(RequestCmd_QRCode2)                     // ��ά��2
#define Request_StreetLight() Request_ToHost(RequestCmd_StreetLight)             // ����·��
#define Request_Garage() Request_ToHost(RequestCmd_Garage)                       // ���峵��
#define Request_TFTShow() Request_ToHost(RequestCmd_TFTShow)                     // TFT��ʾ
#define Request_BarrierGate() Request_ToHost(RequestCmd_BarrierGate)             // ��բ��ʾ
#define Request_LEDShow() Request_ToHost(RequestCmd_LEDShow)                     // LED��־����ʾ
#define Request_PlateRecognition() Request_ToHost(RequestCmd_PlateRecognition)   // ����ʶ��
#define Request_ShapeRecongnition() Request_ToHost(RequestCmd_ShapeRecongnition) // ͼ��ʶ��
#define Request_AGV() Request_ToHost(RequestCmd_AGV)                             // AGV
#define Request_Ultrasonic() Request_ToHost(RequestCmd_Ultrasonic)               // ������
#define Request_Alarm() Request_ToHost(RequestCmd_Alarm)                         // ����̨
#define Request_TrafficLight() Request_ToHost(RequestCmd_TrafficLight)           // ��ͨ��
#define Request_RotatingLED() Request_ToHost(RequestCmd_RotatingLED)             // ��תLED(������ʾ)

// ��������(����Ϊ)
#define Request_GarageFloor() Request_Data(RequestData_GarageFloor) // ���󳵿����
#define Request_Infrared() Request_Data(RequestData_Infrared)       // �����������

// ����ָ��

#define Send_PlateToTFT()                       \
    Send_ZigBeeData(ZigBee_PlateTFT_1, 1, 600); \
    Send_ZigBeeData(ZigBee_PlateTFT_2, 1, 600)

void TFT_Task(void);
void QRCode_Task(void);
void Start_Task(void);
void End_Task(void);
void TrafficLight_Task(void);




// �����
void Task_5_5(void);
void Task_1_5(void);
void Task_3_5(void);
void Task_1_3(void);
void Task_5_3(void);
void Task_5_1(void);
void Task_3_1(void);

#endif // __TASK_H_
