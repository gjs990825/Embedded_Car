#include "task.h"
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "syn7318.h"
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

// ��ͨ��ʶ������
void TrafficLight_Task(void)
{
    Send_ZigBeeData(ZigBee_TrafficLightStartRecognition, 2, 200); // ��ʼʶ��ͨ��
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlag(GetCmdFlag(FromHost_TrafficLight), SET); // �ȴ�ʶ�����
}

// TFTͼ��ʶ������
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition);             // ����ʶ��TFT����
    WaitForFlag(GetCmdFlag(FromHost_TFTRecognition), SET); // �ȴ�ʶ�����
    Request_ToHost(RequestCmd_TFTShow);                    //������ʾ���Ƶ�TFT
}

// ��תled��������
void RotationLED_Task(void)
{
    Infrared_Send_A(Infrared_PlateData1);
    delay_ms(600);
    Infrared_Send_A(Infrared_PlateData2);
}

// ��ά��ʶ��
void QRCode_Task(void)
{
    Request_ToHost(RequestCmd_QRCode1);
    WaitForFlag(GetCmdFlag(FromHost_QRCodeRecognition), SET);
}

// ��ʼ����
void Start_Task(void)
{
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);

    LED_TimerStart();
}

// ��ֹ����
void End_Task(void)
{
    LED_TimerStop();
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);
}

// led��ʾ���루������룩
void LEDDispaly_ShowDistance(uint16_t dis)
{
    ZigBee_LEDDisplayDistanceData[4] = HEX2BCD(dis / 100);
    ZigBee_LEDDisplayDistanceData[5] = HEX2BCD(dis % 100);
    Check_Sum(ZigBee_LEDDisplayDistanceData);
    Send_ZigbeeData_To_Fifo(ZigBee_LEDDisplayDistanceData, 8);
}

// ·�Ƶ�λ���ڣ�����Ŀ�굵λ�Զ�����
void StreetLight_Task(uint8_t targetLevel)
{
    uint16_t temp_val[4], CurrentLightValue;
    int8_t currentLevel, errorValue, i;

    for (i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(5);
        Beep(2);
        Infrared_Send_A(Infrared_LightAdd1);
        delay_ms(700);
        delay_ms(700);
    }

    CurrentLightValue = temp_val[0];

    bubble_sort(temp_val, 4);

    for (i = 0; i < 4; i++)
    {
        if (CurrentLightValue == temp_val[i])
        {
            currentLevel = i + 1;
            break;
        }
    }
    errorValue = targetLevel - currentLevel;
    if (errorValue >= 0)
    {
        for (i = 0; i < errorValue; i++)
        {
            Infrared_Send_A(Infrared_LightAdd1);
            delay_ms(750);
            delay_ms(750);
        }
    }
    else
    {
        for (i = 0; i < 4 + errorValue; i++)
        {
            Infrared_Send_A(Infrared_LightAdd1);
            delay_ms(750);
            delay_ms(750);
        }
    }
}

// ������������Ӧ�����񼯺ϣ������������ǰ��Ҫ��֤λ�þ��볯���׼ȷ����
// ��������Ϳ�ʼ������һ������Ҫ�ֶ����� CurrentStaus.dir = DIR_XX;

void Task_5_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(90 + 50), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 10), Stop_Flag, FORBACKCOMPLETE);
    Beep(2);
    delay_ms(700);
    delay_ms(700); // �ȴ�����ͷ��Ӧ

    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 10), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-50 - 35), Stop_Flag, TURNCOMPLETE);
    Beep(5);
    delay_ms(500);

    RotationLED_Task();

    ExcuteAndWait(Turn_ByEncoder(35), Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT; // ������ʼʱ����һ��
}

extern uint8_t ETC_Flag;

void ETC_Task(void)
{
    u8 count = 0;
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
    while (!ETC_Flag)
    {
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE);
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE);
        count++;
        if (count > 6)
        {
            delay_ms(500);
            ETC_Flag = 1;
            Stop_Flag = Zigb_Rx_Buf[4];
        }
    }
    delay_ms(500);
    delay_ms(500);
    Stop_Flag = Zigb_Rx_Buf[4];
    CurrentStaus.dir = DIR_LEFT;
}

void Task_3_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(22), Stop_Flag, TURNCOMPLETE);
    delay_ms(700);

    TrafficLight_Task();

    ExcuteAndWait(Turn_ByEncoder(-22), Stop_Flag, TURNCOMPLETE);
}

void Task_1_5(void)
{
    delay_ms(700);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

    QRCode_Task();
    LEDDispaly_ShowDistance(Ultrasonic_GetAverage(20));

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
}

void Task_1_3(void)
{
    // ·��// Request_ToHost(RequestCmd_StreetLight);
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

    StreetLight_Task(3);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
}

void Task_5_3(void)
{
    // �����ݻ�
    // SYN7318_Test();
    Beep(4);
}

void Task_5_2(void)
{
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);

    Infrared_Send_A(Infrared_AlarmON);

    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
}

void Task_5_1(void)
{
    // ETC
    ETC_Task();
}

void Task_3_1(void)
{
    // ���
    Send_ZigbeeData_To_Fifo(ZigBee_WirelessChargingON, 8);
    delay_ms(700);
    End_Task();
}
