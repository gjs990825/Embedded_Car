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
// #include "syn7318.h"
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

uint8_t FOUND_RFID_CARD = false;
uint8_t RFID_RoadSection = false;

struct StatusBeforeFoundRFID_Struct
{
    uint8_t stopFlag;
    uint16_t setEncoder;
    uint16_t currentEncoder;
    uint8_t trackMode;
    uint8_t currentSpeed;
    Moving_ByEncoder_t movingByencoder;
} StatusBeforeFoundRFID;

extern uint16_t Mp_Value;
// ���������׿�ʱ���״̬
void Save_StatusBeforeFoundRFID(void)
{
    StatusBeforeFoundRFID.movingByencoder = Moving_ByEncoder;
    StatusBeforeFoundRFID.currentEncoder = Mp_Value;
    StatusBeforeFoundRFID.stopFlag = Stop_Flag;
    StatusBeforeFoundRFID.trackMode = Track_Mode;
    StatusBeforeFoundRFID.setEncoder = temp_MP;
    StatusBeforeFoundRFID.currentSpeed = Car_Speed;
}

// �ָ�״̬ encoderChangeValue: ǰ���趨���̲�ֵ
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue)
{
    Roadway_mp_syn(); // ͬ������
    Moving_ByEncoder = StatusBeforeFoundRFID.movingByencoder;
    Stop_Flag = StatusBeforeFoundRFID.stopFlag;
    Track_Mode = StatusBeforeFoundRFID.trackMode;
    // ѭ����Ϣ����գ���Ҫ���¼��㲢��ȥִ���е��н�ֵ
    temp_MP = StatusBeforeFoundRFID.setEncoder - StatusBeforeFoundRFID.currentEncoder - encoderChangeValue;
    Car_Speed = StatusBeforeFoundRFID.currentSpeed;
}

// ��ͨ��ʶ��
void TrafficLight_Task(void)
{
    Send_ZigBeeDataNTimes(ZigBee_TrafficLightStartRecognition, 2, 200); // ��ʼʶ��ͨ��
    delay_ms(700);
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlag(GetCmdFlag(FromHost_TrafficLight), SET); // �ȴ�ʶ�����
}

// TFTͼ��ͼ��ʶ��
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition);             // ����ʶ��TFT����
    WaitForFlag(GetCmdFlag(FromHost_TFTRecognition), SET); // �ȴ�ʶ�����
    Request_ToHost(RequestCmd_TFTShow);                    //������ʾ���Ƶ�TFT
}

// ��תled���ͳ���
void RotationLED_Task(void)
{
    Request_ToHost(RequestCmd_RotatingLED);
    delay_ms(790);
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
    Send_ZigBeeData(ZigBee_LEDDisplayDistanceData);
}

// ·�Ƶ�λ���ڣ�����Ŀ�굵λ�Զ�����
void StreetLight_Task(uint8_t targetLevel)
{
    uint16_t temp_val[4], CurrentLightValue;
    int8_t errorValue, i;

    for (i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Beep(2);
        Infrared_Send_A(Infrared_LightAdd1);
        delay_ms(790);
        delay_ms(790);
    }
    CurrentLightValue = temp_val[0];

    bubble_sort(temp_val, 4); // �Ի���������������ʼ��λ
    for (i = 0; i < 4; i++)
    {
        if (CurrentLightValue == temp_val[i])
        {
            errorValue = (int8_t)targetLevel - (i + 1);
            break;
        }
    }

    if (errorValue >= 0) // ������Ŀ�굵λ
    {
        for (i = 0; i < errorValue; i++)
        {
            Infrared_Send_A(Infrared_LightAdd1);
            delay_ms(790);
            delay_ms(790);
        }
    }
    else
    {
        for (i = 0; i > errorValue; i--)
        {
            Infrared_Send_A(Infrared_LightAdd3);
            delay_ms(790);
            delay_ms(790);
        }
    }
}

// ETC����
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // ҡ��10�Σ�����ֱ����
    {
        if ((ETC_Status.isSet == SET) && Check_IsTimeOut(ETC_Status.timeStamp, 6 * 1000)) // ����ǰ����������
            break;
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 7), Stop_Flag, FORBACKCOMPLETE); // ���Ž���
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 7), Stop_Flag, FORBACKCOMPLETE); // һ��ҡ��
    }
}

// RFID��������⵽ִ�ж���
void RFID_Task(void)
{
    print_info("FOUND_RFID\r\n");
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 12), Stop_Flag, FORBACKCOMPLETE);
    for (uint8_t i = 0; i < 5; i++) // ������ΧԼ 11.5-16.5�������ȡ
    {
        Read_Card();
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 1), Stop_Flag, FORBACKCOMPLETE);
        delay_ms(500);
    }
    RFID_RoadSection = false; // ����Ѱ��
    FOUND_RFID_CARD = false;  // ��ձ�־λ
    TIM_Cmd(TIM5, DISABLE);   // ֹͣ��ʱ��
    ExcuteAndWait(Back_Off(30, Centimeter_Value * (12 + (1 * 5))), Stop_Flag, FORBACKCOMPLETE);
    Control(Car_Speed, Car_Speed); 
    // ���ض���ǰλ��
}

// ��բ����(���복�Ƶ��ַ���)
void BarrierGate_Task(uint8_t plate[6])
{
    memcpy(&ZigBee_PlateBarrierGate_1[3], plate, 3);
    memcpy(&ZigBee_PlateBarrierGate_2[3], &plate[3], 3);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_1);
    delay_ms(790);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_2);
    delay_ms(790);
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
}

// void AGV_Task(void)
// {
//     AGV_Stop();
// }

// ������������Ӧ�����񼯺ϣ������������ǰ��Ҫ��֤λ�þ��볯���׼ȷ����
// ��������Ϳ�ʼ������һ������Ҫ�ֶ����� CurrentStaus.dir = DIR_XX;

void Task_5_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(90 + 50), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
    Beep(2);

    delay_ms(700);
    delay_ms(700); // �ȴ�����ͷ��Ӧ
    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-50 - 35), Stop_Flag, TURNCOMPLETE);

    RotationLED_Task();

    ExcuteAndWait(Turn_ByEncoder(35), Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT; // ������ʼʱ����һ��
}

void Task_3_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(22), Stop_Flag, TURNCOMPLETE);

    TrafficLight_Task();

    ExcuteAndWait(Turn_ByEncoder(-22), Stop_Flag, TURNCOMPLETE);

    RFID_RoadSection = true; // �׿�·�ο�ʼ
}

void Task_1_5(void)
{
    ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

    delay_ms(700);
    QRCode_Task();

    uint16_t distanceMeasured = Ultrasonic_GetAverage(20);
	LEDDispaly_ShowDistance(distanceMeasured); // �����η�ֹ����
    delay_ms(700);
    LEDDispaly_ShowDistance(distanceMeasured);

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
}

void Task_1_3(void)
{
    RFID_RoadSection = false; // �׿�·�ν���

    // ·��
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
    ExcuteAndWait(Turn_ByEncoder(90 + 45), Stop_Flag, TURNCOMPLETE);

    Infrared_Send_A(Infrared_AlarmON);

    ExcuteAndWait(Turn_ByEncoder(-45), Stop_Flag, TURNCOMPLETE);

    AGV_SetTowards(DIR_DOWN);
    delay_ms(700);
    BarrierGate_Task("A12B34");
    Send_ZigBeeData(ZigBee_AGVStart);

    CurrentStaus.dir = DIR_DOWN;
}

void Task_5_1(void)
{
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);

    ETC_Task();

    CurrentStaus.dir = DIR_LEFT;
}

void Task_3_1(void)
{
    // ���
    delay_ms(500);
    Send_ZigBeeData(ZigBee_WirelessChargingON);
    delay_ms(700);
    End_Task();
}
