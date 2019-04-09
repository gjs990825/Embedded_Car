#include "independent_task.h"
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
#include "Timer.h"


// Ѱ���׿�
uint8_t FOUND_RFID_CARD = false;
// �׿�·��
uint8_t RFID_RoadSection = false;
// �׿�����
uint8_t RFID_x = 0, RFID_y = 0;
// �׿����ݿ�λ��
uint8_t RFID_DataBlockLoation = 5;
// �׿����ݴ洢
uint8_t RFID_DataBuffer[17] = {0};

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

// �趨Ϊ�׿�·��
void Task_RFID_RoadSectionTrue(void)
{
    RFID_RoadSection = true;
}

// �趨Ϊ�ǰ׿�·��
void Task_RFID_RoadSectionFalse(void)
{
    RFID_RoadSection = false;
}

// RFID��������⵽ִ�ж���
void RFID_Task(void)
{
    uint8_t i;
    RFID_x = NextStatus.x; // ��ȡRFIDλ��
    RFID_y = NextStatus.y;

    print_info("FOUND_RFID:%d,%d\r\n", RFID_x, RFID_y);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE);
    for (i = 0; i < 9; i++) // ������ΧԼ 11.5-16.5�������ȡ()
    {
        if (Read_RFID_Block(RFID_DataBlockLoation, RFID_DataBuffer) == true)
            break; // ��ȡ�ɹ�������
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 1), Stop_Flag, FORBACKCOMPLETE);
        delay_ms(500);
    }

    RFID_RoadSection = false; // ����Ѱ��
    FOUND_RFID_CARD = false;  // ��ձ�־λ
    TIM_Cmd(TIM5, DISABLE);   // ֹͣ��ʱ��

    ExcuteAndWait(Back_Off(30, Centimeter_Value * (8 + i)), Stop_Flag, FORBACKCOMPLETE); // ���ض���ǰλ��
    // ʮ��·����Ҫ���˺�һ�㣬��Ϊ��Ӧʱ��䳤�����һ��
    if (StatusBeforeFoundRFID.stopFlag == CROSSROAD)
    {
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 2), Stop_Flag, FORBACKCOMPLETE);
    }

    Update_MotorSpeed(Car_Speed, Car_Speed);
}

void Test_RFID(uint8_t block)
{
    uint8_t buf[17];
    Read_RFID_Block(block, buf);
}

// ������ ʹ��KEYA������buf��
bool Read_RFID_Block(uint8_t block, uint8_t *buf)
{
    uint8_t key[8]; // ʹ��Ĭ��key

    for (uint8_t i = 0; i < 8; i++)
    {
        key[i] = 0xFF;
    }
    if (RFID_ReadBlock(block, key, buf) == MI_OK)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%X ", buf[i]);
        }
        print_info("\r\n");
        return true;
    }
    else
    {
        print_info("ERROR\r\n");
        return false;
    }
}


// ��ͨ��ʶ��
void TrafficLight_Task(void)
{
    Send_ZigBeeDataNTimes(ZigBee_TrafficLightStartRecognition, 2, 200); // ��ʼʶ��ͨ��
    delay_ms(700);
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlagInMs(GetCmdFlag(FromHost_TrafficLight), SET, 13 * 1000); // �ȴ�ʶ�����
}

// TFTͼ��ͼ��ʶ��
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition);                            // ����ʶ��TFT����
    WaitForFlagInMs(GetCmdFlag(FromHost_TFTRecognition), SET, 37 * 1000); // �ȴ�ʶ�����
}


// TFT��ʾ����
// TFT��ʾHEX
void TFT_Hex(uint8_t dat[3])
{
    uint8_t buf[8] = {0x55, 0x0b, 0x10, 0x02, 0x00, 0x00, 0x12, 0xbb};
    buf[2] = 0x40;
    memcpy(&buf[3], dat, 3);
    Send_ZigBeeData(buf);
}

// ������ʾ����

// ������ʾ ��ʾ����
void RotationLED_Plate(uint8_t plate[6], uint8_t coord[2])
{
    // Infrared_PlateData1[]
    memcpy(&Infrared_PlateData1[2], plate, 4);
    memcpy(&Infrared_PlateData2[2], &plate[4], 2);
    memcpy(&Infrared_PlateData2[4], coord, 2);
    Infrared_Send_A(Infrared_PlateData1);
    delay_ms(600);
    Infrared_Send_A(Infrared_PlateData2);
}

// ������ʾ ��ʾ����
void RotationLED_Distance(uint8_t dis)
{
    uint8_t buf[6] = {0x00};

    buf[0] = 0xFF;
    buf[1] = 0x11;
    buf[2] = HEX2BCD((dis % 100) / 10);
    buf[2] = HEX2BCD(dis % 10);
    Infrared_Send_A(buf);
}

// ��ά��ʶ��
void QRCode_Task(uint8_t QRrequest)
{
    GetCmdFlag(FromHost_QRCodeRecognition) = RESET;
    Request_ToHost(QRrequest);
    WaitForFlagInMs(GetCmdFlag(FromHost_QRCodeRecognition), SET, 5 * 1000);
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
void StreetLight_AdjustTo(uint8_t targetLevel)
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


// ��բ��ʾ����
void BarrierGate_Plate(uint8_t plate[6])
{
    memcpy(&ZigBee_PlateBarrierGate_1[3], plate, 3);
    memcpy(&ZigBee_PlateBarrierGate_2[3], &plate[3], 3);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_1);
    delay_ms(790);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_2);
    delay_ms(790);
}

// ��բ����
void BarrierGate_Task(uint8_t plate[6])
{
    if (plate != NULL)
    {
        BarrierGate_Plate(plate);
    }
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
}

void Voice_Task(void)
{
    Start_VoiceCommandRecognition(3);
}

// ETC����
void ETC_Task(void)
{
    // ��ʱ�߼���Ҫ����
    for (uint8_t i = 0; i < 10; i++) // ҡ��10�Σ�����ֱ����
    {
        // ����ǰ����������
        if ((ETC_Status.isSet == SET) && (!IsTimeOut(ETC_Status.timeStamp, 6 * 1000)))
            break;
        MOVE(7);  // ���Ž���
        MOVE(-7); // һ��ҡ��
    }
}
