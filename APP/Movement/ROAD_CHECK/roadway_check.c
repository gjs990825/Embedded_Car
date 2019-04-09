#include "stm32f4xx.h"
#include "CanP_Hostcom.h"
#include "delay.h"
#include "roadway_check.h"
#include "cba.h"
#include "tba.h"
#include "movement.h"
#include "debug.h"
#include "hardware.h"
#include "pid.h"
#include "my_lib.h"

// ת���Ƿ����
#define CheckTurnComplete(EncoderValue) \
    if (Mp_Value >= EncoderValue)       \
    {                                   \
        Stop();                         \
        Stop_Flag = TURNCOMPLETE;       \
    }

// ʹ��ѭ����Ϣ���
#define _TRACK_OUTPUT_ 0

// ���������ж�����/�����׿�
#define ALL_WHITE 15
// ���ڴ����ж�ײ��
#define ALL_BLACK 7

// ǰ�߸�ѭ��������
int8_t Q7[7] = {0};
// ��˸�ѭ��������
int8_t H8[8] = {0};
// ��ǰ��ɫ����
uint8_t NumberOfWhite = 0;
// ����Ȩ��(PID��Ӧ����ֵ�������Ѹ���Ϊƫ��ֵ)
int DirectionWights = 0;
// ����״̬
uint8_t Stop_Flag = TRACKING;
// ѭ��ģʽ
TrackMode_t Track_Mode = TrackMode_NONE;
// ��ֵǰ���ת��
Moving_ByEncoder_t Moving_ByEncoder = ENCODER_NONE;
// ���Ƕ�ת��Ŀ������ֵ
uint16_t TurnByEncoder_Value = 0;

// �����ٶ�
int LSpeed = 0, RSpeed = 0;
// ѭ��ʱ����
int Car_Speed = 0;

// Ŀ������ֵ
uint16_t temp_MP = 0;
// ��ǰ������ֵ
uint16_t Mp_Value = 0;

// ֮ǰ��ȡ������ֵ
int16_t Roadway_cmp;
// ��CAN��ȡ��ʵʱ����ֵ
extern int16_t CanHost_Mp;

// ����ͬ��
void Roadway_mp_syn(void)
{
    Roadway_cmp = CanHost_Mp;
}

// ���̻�ȡ
uint16_t Roadway_mp_Get(void)
{
    uint32_t ct;
    if (CanHost_Mp > Roadway_cmp)
        ct = CanHost_Mp - Roadway_cmp;
    else
        ct = Roadway_cmp - CanHost_Mp;
    if (ct > 0x8000)
        ct = 0xffff - ct;

    return ct;
}

uint16_t Roadway_Navig;
extern uint16_t CanHost_Navig;

// �Ƕ�ͬ��
void Roadway_nav_syn(void)
{
    Roadway_Navig = CanHost_Navig;
}

// ��ȡ�ǶȲ�ֵ
uint16_t Roadway_nav_Get(void)
{
    uint16_t ct;
    if (CanHost_Navig > Roadway_Navig)
        ct = CanHost_Navig - Roadway_Navig;
    else
        ct = Roadway_Navig - CanHost_Navig;
    while (ct >= 36000)
        ct -= 36000;
    return ct;
}

// �����־λ
void Roadway_Flag_clean(void)
{
    Stop_Flag = TRACKING;
    temp_MP = 0;
    Track_Mode = TrackMode_NONE;
    Moving_ByEncoder = ENCODER_NONE;
}

// ǰ��ת����
void Moving_ByEncoderCheck(void)
{
    switch (Moving_ByEncoder)
    {
    case ENCODER_GO:
        if (temp_MP <= Roadway_mp_Get())
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
        }
        break;
    case ENCODER_BACK:
        if (temp_MP <= Roadway_mp_Get())
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
        }
        break;
    case ENCODER_TurnByValue:
        CheckTurnComplete(TurnByEncoder_Value);
    default:
        break;
    }
}

// �����ٶ�ֵ���ٶ����� -100 ~ 100)
void Update_MotorSpeed(int L_Speed, int R_Speed)
{
    // �ٶ��޷�
    LSpeed = constrain_int(L_Speed, -100, 100);
    RSpeed = constrain_int(R_Speed, -100, 100);
}

// ������ƣ��ύ����ٶȸ���
void Submit_SpeedChanges(void)
{
    static int preLSpeed, preRSpeed;  // �ϴε��ٶ�����
    static uint32_t preSpeedChanging; // �ϴα���ٶȵ�ʱ���

    // �ٶ�ֵ�ı����ϴ�����
    if (LSpeed != preLSpeed || RSpeed != preRSpeed)
    {
        preLSpeed = LSpeed;
        preRSpeed = RSpeed;
        Send_UpMotor(LSpeed, RSpeed);
        preSpeedChanging = Get_GlobalTimeStamp(); // ����ʱ���
    }
    else
    {
        // ���һ��ʱ�����һ���ٶ���Ϣ����ֹcan�������������ʻʱ�����ش���
        if (IsTimeOut(preSpeedChanging, 150))
        {
            Send_UpMotor(LSpeed, RSpeed);
            preSpeedChanging = Get_GlobalTimeStamp();
        }
    }
}

// ��ȡѭ����Ϣ�������ɫ������
void Get_Track(void)
{
    uint16_t tmp = Get_Host_UpTrack(TRACK_ALL);

    NumberOfWhite = 0;

    for (uint8_t i = 0; i < 7; i++)
    {
        Q7[i] = (tmp >> i) & 0x01;
        NumberOfWhite += Q7[i] ? 1 : 0;
        H8[i] = (tmp >> (8 + i)) & 0x01;
        NumberOfWhite += H8[i] ? 1 : 0;
    }
    H8[7] = (tmp >> (15)) & 0x01;
    NumberOfWhite += H8[7] ? 1 : 0; // edited

#if _TRACK_OUTPUT_

    for (int i = 6; i >= 0; i--)
    {
        print_info("%d", Q7[i] ? 1 : 0);
    }
    print_info("\r\n");
    for (int i = 7; i >= 0; i--)
    {
        print_info("%d", H8[i] ? 1 : 0);
    }
    print_info("\r\n");
    print_info("N_O_W_:%d\r\n", NumberOfWhite);

#endif // _TRACK_OUTPUT_
}

// ���㷽��Ȩ��
void Get_DirectionWights(void)
{

    DirectionWights = 0;

    for (uint8_t i = 1; i <= 4; i++)
    {
        DirectionWights += (H8[3 + i] - H8[4 - i]);
    }
    for (uint8_t i = 1; i <= 4; i++)
    {
        DirectionWights += (Q7[2 + i] - Q7[4 - i]);
    }

    Calculate_pid(DirectionWights);

#if _TRACK_OUTPUT_

    print_info("T:%d\r\n", DirectionWights);

#endif // _TRACK_OUTPUT_
}

uint8_t TrackStatus = 0;
uint8_t isOutTrack = false;
uint32_t outTrackStamp;

// ѭ��
void TRACK_LINE(void)
{

    if (Track_Mode == TrackMode_ENCODER)
    {
        if (temp_MP < Mp_Value)
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
            return;
        }
    }

    Get_Track();
    Get_DirectionWights();

    if (Track_Mode == TrackMode_Turn) // ͨ��ѭ����ת��Ч�����Ǻܺã��������ã�
    {
        if (TrackStatus == 0)
        {
            if (NumberOfWhite >= ALL_WHITE)
            {
                TrackStatus = 1;
                outTrackStamp = Get_GlobalTimeStamp();
            }
        }
        else if (TrackStatus == 1)
        {
            if ((NumberOfWhite < ALL_WHITE) && IsTimeOut(outTrackStamp, 200))
            {
                TrackStatus = 2;
                PidData_Clear();
            }
        }
        else if (TrackStatus == 2)
        {
            if (PID_value < 20 && PID_value > -20)
            {
                TrackStatus = 0;
                Stop();
                Stop_Flag = TURNCOMPLETE;
                return;
            }
        }
        return;
    }

    if (NumberOfWhite >= ALL_WHITE) // ȫ��
    {
        if ((Track_Mode == TrackMode_NORMAL) || (Track_Mode == TrackMode_ENCODER)) // ѭ��״̬
        {
            if (isOutTrack == false)
            {
                isOutTrack = true;
                outTrackStamp = Get_GlobalTimeStamp();
            }
            else
            {
                if (IsTimeOut(outTrackStamp, 60)) // ��ȫ�׳���60ms
                {
                    if (RFID_RoadSection) // �׿�·��
                    {
                        FOUND_RFID_CARD = true;       // �ҵ��׿�
                        Save_StatusBeforeFoundRFID(); // ���浱ǰ״̬
                        Stop();                       // ��ͣ����
                        TIM_Cmd(TIM5, ENABLE);        // ʹ��RFID����ʱ��
                    }
                    else
                    {
                        // Stop();
                        Stop_Flag = OUTTRACK; // ����
                    }
                }
            }
        }
    }
    else if ((NumberOfWhite <= ALL_BLACK) && (Track_Mode == TrackMode_NORMAL)) // ȫ��
    {
        Roadway_Flag_clean();
        Update_MotorSpeed(0, 0);
        Stop_Flag = CROSSROAD;
        isOutTrack = false;
    }
    else
    {
        if (RFID_RoadSection && (FOUND_RFID_CARD == false)) // �׿�·�Σ���һ��������
        {
            // �ж�ѭ�����Ƿ��з��׵���� // ���м���λ����Ϊ��ɫ��������������һ����ɫ��������һ����Ϊ·��
            if (((H8[0] + Q7[0] + H8[7] + Q7[6]) <= 3) && (H8[3] & H8[4] & Q7[3]) && (((NextStatus.x % 2) && (NextStatus.y % 2)) != 0))
            {
                Roadway_Flag_clean(); // ��������ʮ���ߵ����
                Update_MotorSpeed(0, 0);
                Stop_Flag = CROSSROAD;
                FOUND_RFID_CARD = true;       // �ҵ��׿�
                Save_StatusBeforeFoundRFID(); // ���浱ǰ״̬
                Stop();                       // ��ͣ����
                TIM_Cmd(TIM5, ENABLE);        // ʹ��RFID����ʱ��
            }
            else
            {
                Update_MotorSpeed(Car_Speed + PID_value, Car_Speed - PID_value);
            }
        }
        else
        {
            Update_MotorSpeed(Car_Speed + PID_value, Car_Speed - PID_value);
            isOutTrack = false;
            // ��CAN���ͳ��ֹ�û���ʹ�������ٶȿ����ڶ�ʱ���ж���ʵ��
        }
    }
}

// �˶�����
void Roadway_Check(void)
{
    if (Track_Mode != TrackMode_NONE)
    {
        TRACK_LINE();
    }
    if (Moving_ByEncoder != ENCODER_NONE)
    {
        Moving_ByEncoderCheck();
    }
    
    Submit_SpeedChanges();
}

// ·����⣨TIM9��
void Roadway_CheckTimInit(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

    TIM_InitStructure.TIM_Period = arr;
    TIM_InitStructure.TIM_Prescaler = psc;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM9, &TIM_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
}

extern uint32_t lastStopStamp;
void TIM1_BRK_TIM9_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        // DEBUG_PIN_2_SET();

        // ��һ��ֹͣʱ��δ�ȴ��㹻ʱ���򲻽�����һ����������ֹ��
        if (IsTimeOut(lastStopStamp, 300))
        {
            Mp_Value = Roadway_mp_Get();
            Roadway_Check();
        }
        // DEBUG_PIN_2_RESET();
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
}
