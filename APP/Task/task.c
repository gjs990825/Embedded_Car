#include "task.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "voice.h"
#include "power_check.h"
#include "can_user.h"
#include "data_base.h"
#include "roadway_check.h"
#include "tba.h"
#include "data_base.h"
#include "swopt_drv.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "delay.h"
#include "can_user.h"
#include "Timer.h"
#include "Rc522.h"
#include "malloc.h"
#include "a_star.h"
#include "debug.h"
#include "movement.h"
#include "protocol.h"
#include "hardware.h"
#include "route.h"
#include "ultrasonic.h"
#include "my_lib.h"
#include "seven_seg.h"
#include "Timer.h"
#include "agv.h"


// ������������Ӧ�Ķ������񼯺ϣ������������ǰ��Ҫ��֤λ�þ��볯���׼ȷ����
// ��������Ϳ�ʼ������һ������Ҫ�ֶ����� CurrentStaus.dir = DIR_XX;

void Task_5_0(void)
{
    Start_Task();
}

uint8_t *RFID_Key = NULL;
void Task_F6(void)
{
    TURN(-50);
    MOVE(15);

    delay_ms(700);
    QRCode_Task(RequestCmd_QRCode1);
    RFID_Key = Get_QRCode(DataRequest_QRCode1, 0x01); // ��ȡ��Կ

    MOVE(-15);
    TURN(-40);

//    RFID_DataBlockLoation = Get_QRCode(DataRequest_QRCode1, 0x02)[0]; // ��ȡ��ά����Ϣ�е�RFID���ݿ���Ϣ
//    print_info("RFID_Block:%d\r\n", RFID_DataBlockLoation);

    CurrentStaus.dir = DIR_LEFT;
}

void Task_3_1(void)
{
    TrafficLight_Task();
    RFID_RoadSection = true; // �׿�·�ο�ʼ
}

uint16_t distanceMeasured = 0;

void Task_1_3(void)
{
    ExcuteAndWait(Turn_ByEncoder(-93), Stop_Flag, TURNCOMPLETE); // ����ֵ
    ExcuteAndWait(Back_Off(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);

    delay_ms(700);
    QRCode_Task(RequestCmd_QRCode2);
    AGV_SendInfraredData(Get_QRCode(DataRequest_QRCode2, 0x00)); // ���ͺ������ݵ��ӳ�

    distanceMeasured = Ultrasonic_GetAverage(20);
    LEDDispaly_ShowDistance(distanceMeasured); // �����η�ֹ����
    delay_ms(700);
    LEDDispaly_ShowDistance(distanceMeasured);

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(93), Stop_Flag, TURNCOMPLETE); // ����ֵ
}

void Task_1_5(void)
{
    RFID_RoadSection = false;

    ExcuteAndWait(Turn_ByEncoder(-40), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(40), Stop_Flag, TURNCOMPLETE);

    TFT_Hex(Get_ShapeInfo());
    delay_ms(790);
    TFT_Hex(Get_ShapeInfo());
    delay_ms(790);

    AGV_SendInfraredData(Infrared_AlarmON); // ���ͺ�����Ϣ // ��Ҫע��������
    delay_ms(700);
//    AGV_SetTaskID(Get_TaskNumber("D4", RFID_DataBuffer, 1), 0); // �趨�����
//    AGV_SetRoute(RFID_DataBuffer);                              // ���ʹӳ�·����Ϣ
    delay_ms(700);
    AGV_SetTowards(DIR_LEFT); // �趨��ͷ����
    delay_ms(700);            // �ȴ�
    AGV_Start();              // �ӳ�����
    delay_ms(700);            // �ȴ�
    AGV_Start();

//    if (Get_TaskNumber("B1", RFID_DataBuffer, 1) != -1) // �ӳ���������趨
//    {
//        AGV_SetTaskID(Get_TaskNumber("B1", RFID_DataBuffer, 1), 1);
//    }
//    else if (Get_TaskNumber("B7", RFID_DataBuffer, 1) != -1)
//    {
//        AGV_SetTaskID(Get_TaskNumber("B7", RFID_DataBuffer, 1), 1);
//    }

//    if (Get_TaskNumber("B2", RFID_DataBuffer, 1) != -1) // ����������·��
//    {
//        ExcuteAndWait(Back_Off(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
//    }

//    AGVComplete_Status.isSet = RESET;
//    WaitForFlagInMs(AGVComplete_Status.isSet, SET, 25 * 1000); // �ȴ��ӳ�ִ��������

//    if (Get_TaskNumber("B2", RFID_DataBuffer, 1) != -1) // ����������·��
//    {
//        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
//    }

    ExcuteAndWait(Turn_ByEncoder(90 + 35), Stop_Flag, TURNCOMPLETE);

//    RotationLED_Plate(Get_PlateNumber(), ReCoordinate_Covent(RFID_x, RFID_y));

    ExcuteAndWait(Turn_ByEncoder(-35), Stop_Flag, TURNCOMPLETE);

    CurrentStaus.dir = DIR_RIGHT;
}

void Task_3_5(void)
{
    uint8_t level = (Get_AllColorCount() * (distanceMeasured / 100)) % 4 + 1;

    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

    StreetLight_AdjustTo(level);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
}

void Task_5_5(void)
{
    BarrierGate_Task(NULL);
}

void Task_3_1_2(void)
{
    Voice_Task();
}

void Task_F6_2(void)
{
    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 30), Stop_Flag, FORBACKCOMPLETE);

    Send_ZigBeeData(ZigBee_WirelessChargingON);
    Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 2, 700);
    End_Task();
}

// void Task_5_5(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(90 + 50), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
//     Beep(2);

//     delay_ms(700);
//     delay_ms(700); // �ȴ�����ͷ��Ӧ
//     TFT_Task();

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(-50 - 35), Stop_Flag, TURNCOMPLETE);

//     RotationLED_Task();

//     ExcuteAndWait(Turn_ByEncoder(35), Stop_Flag, TURNCOMPLETE);
//     CurrentStaus.dir = DIR_LEFT; // ������ʼʱ����һ��
// }

// void Task_3_5(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(22), Stop_Flag, TURNCOMPLETE);

//     TrafficLight_Task();

//     ExcuteAndWait(Turn_ByEncoder(-22), Stop_Flag, TURNCOMPLETE);

//     RFID_RoadSection = true; // �׿�·�ο�ʼ
// }

// void Task_1_5(void)
// {
//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

//     delay_ms(700);
//     QRCode_Task();

//     uint16_t distanceMeasured = Ultrasonic_GetAverage(20);
//     LEDDispaly_ShowDistance(distanceMeasured); // �����η�ֹ����
//     delay_ms(700);
//     LEDDispaly_ShowDistance(distanceMeasured);

//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
// }

// void Task_1_3(void)
// {
//     RFID_RoadSection = false; // �׿�·�ν���

//     // ·��
//     ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

//     StreetLight_AdjustTo(3);

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
// }

// void Task_5_3(void)
// {
//     Voice_Task();

//     ExcuteAndWait(Turn_ByEncoder(90 + 45), Stop_Flag, TURNCOMPLETE);

//     Infrared_Send_A(Infrared_AlarmON);

//     ExcuteAndWait(Turn_ByEncoder(-45), Stop_Flag, TURNCOMPLETE);

//     AGV_SetTowards(DIR_DOWN);
//     delay_ms(700);
//     BarrierGate_Task("A12B34");
//     Send_ZigBeeData(ZigBee_AGVStart);

//     CurrentStaus.dir = DIR_DOWN;
// }

// void Task_F6(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);

//     ETC_Task();

//     CurrentStaus.dir = DIR_LEFT;
// }

// void Task_3_1(void)
// {
//     // ���
//     delay_ms(500);
//     Send_ZigBeeData(ZigBee_WirelessChargingON);
//     delay_ms(700);
//     End_Task();
// }
