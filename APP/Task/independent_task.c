#include "independent_task.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "bh1750.h"
#include "voice.h"
#include "roadway_check.h"
#include "tba.h"
#include "data_base.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "Rc522.h"
#include "malloc.h"
#include "debug.h"
#include "movement.h"
#include "route.h"
#include "my_lib.h"
#include "Timer.h"
#include "data_from_host.h"

// RFID��� ��

// Ѱ���׿�
uint8_t FOUND_RFID_CARD = false;
// �׿�·��
uint8_t RFID_RoadSection = false;
// ��ǰ����Ϣָ��
RFID_Info_t *CurrentRFIDCard = NULL;
// �����׿�ʱ��״̬����
struct StatusBeforeFoundRFID_Struct
{
    uint8_t stopFlag;
    uint16_t setEncoder;
    uint16_t currentEncoder;
    uint8_t trackMode;
    uint8_t currentSpeed;
    Moving_ByEncoder_t movingByencoder;
} StatusBeforeFoundRFID;

// ���������׿�ʱ���״̬
void Save_StatusBeforeFoundRFID(void)
{
    extern uint16_t Mp_Value;

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

// �趨��ǰ����Ϣ
void Set_CurrentCardInfo(RFID_Info_t *RFIDx)
{
    CurrentRFIDCard = RFIDx;
}

// ���õ��Կ�ʹ�õ���Ϣ
uint8_t _testRFIDDataBlock = 5;
uint8_t _testRFIDKey[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t _testRFIDAuthMode = PICC_AUTHENT1A;

// RFID��������ʼ
void Task_RFIDTestStart(void)
{
    RFID_Info_t *rfid = mymalloc(SRAMIN, sizeof(RFID_Info_t));

    // ���
    memset(rfid, 0, sizeof(RFID_Info_t));

    // д�������Ϣ
    memcpy(rfid->key, _testRFIDKey, 6);
    rfid->authMode = _testRFIDAuthMode;
    rfid->dataBlockLocation = _testRFIDDataBlock;

    Set_CurrentCardInfo(rfid);
    RFID_RoadSection = true;
}

// RFID�����������
void Task_RFIDTestEnd(void)
{
    myfree(SRAMIN, CurrentRFIDCard);
    CurrentRFIDCard = NULL;
    RFID_RoadSection = false;
}

// ʹ���趨key��ĳ������
void Test_RFID(uint8_t block)
{
    uint8_t buf[17];

    if (PICC_ReadBlock(block, _testRFIDAuthMode, _testRFIDKey, buf) == SUCCESS)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%02X ", buf[i]);
            delay_ms(5);
        }
        print_info("\r\n");
    }
    else
    {
        print_info("ERROR\r\n");
    }
}

// ����
ErrorStatus Read_RFID(RFID_Info_t *RFIDx)
{
    ErrorStatus status = PICC_ReadBlock(RFIDx->dataBlockLocation, RFIDx->authMode, RFIDx->key, RFIDx->data);

    if (status == SUCCESS)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%02X ", RFIDx->data[i]);
            delay_ms(5);
        }
        print_info("\r\n");
    }
    else
    {
        print_info("READ CARD FAIL\r\n");
    }
    return status;
}

// RFID�������񣬼�⵽�׿�ʱִ��
void RFID_Task(void)
{
    uint8_t i;

    // ��ǰ����Ϣδ�趨������
    if (CurrentRFIDCard == NULL)
        return;

    // ��¼λ����Ϣ
    CurrentRFIDCard->coordinate = NextStatus;
    print_info("Card At:(%d,%d)\r\n", CurrentRFIDCard->coordinate.x, CurrentRFIDCard->coordinate.y);

    MOVE(8);
    for (i = 0; i < 9; i++) // ������ΧԼ 11.5-16.5�����һ���ֶ�ȡ()
    {
        if (Read_RFID(CurrentRFIDCard) == SUCCESS)
            break; // ��ȡ�ɹ�������
        MOVE(1);
        delay_ms(500);
    }

    RFID_RoadSection = false; // ����Ѱ��
    FOUND_RFID_CARD = false;  // ��ձ�־λ
    TIM_Cmd(TIM5, DISABLE);   // ֹͣ��ʱ��
    MOVE(-(8 + i));           // ���ض���ǰλ��

    // ʮ��·����Ҫ���˺�һ�㣬��Ϊ��Ӧʱ��䳤�����һ��
    if (StatusBeforeFoundRFID.stopFlag == CROSSROAD)
    {
        MOVE(-2);
    }

    Update_MotorSpeed(Car_Speed, Car_Speed);
}

// RFID��� ��

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
void RotationLED_Distance(uint16_t dis)
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

    Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStartTimer, 3, 20);
}

// ��ֹ����
void End_Task(void)
{
    Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStopTimer, 3, 20);
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

// �������񣬴�������2��
void Voice_Task(void)
{
    Start_VoiceCommandRecognition(3);
}

// ETC����
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // ����10�Σ�����ֱ����
    {
        // ����ǰ����������
        if ((ETC_Status.isSet == SET) && (!IsTimeOut(ETC_Status.timeStamp, 6 * 1000)))
            break;
        MOVE(7);  // ���Ž���
        MOVE(-7); // һ��ҡ��
    }
}
