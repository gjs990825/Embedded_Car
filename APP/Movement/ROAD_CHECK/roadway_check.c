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

#define _TRACK_OUTPUT_ 0

#define ALL_WHITE 15
#define ALL_BLACK 7
#define WEIGHTS_MULTIPLE 6

// Gao added
int8_t Q7[7] = {0};
int8_t H8[8] = {0};
// 白色数量
uint8_t NumberOfWhite = 0;
// 方向权重
int DirectionWights = 0;

// 运行状态
uint8_t Stop_Flag = TRACKING;

// 循迹模式
uint8_t Track_Mode = TrackMode_NONE;

// 定值前后和转向
Moving_ByEncoder_t Moving_ByEncoder = ENCODER_NONE;

// 左右速度
int LSpeed = 0, RSpeed = 0;
// 循迹时车速
int Car_Speed = 0;

// 设定码盘值
uint16_t temp_MP = 0;
// 获取的码盘值
uint16_t Mp_Value = 0;

// 码盘中间变量
int16_t Roadway_cmp;
extern int16_t CanHost_Mp;

// 码盘同步
void Roadway_mp_syn(void)
{
    Roadway_cmp = CanHost_Mp;
}

uint16_t Roadway_mp_Get(void) //码盘获取
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

// 角度同步
void Roadway_nav_syn(void)
{
    Roadway_Navig = CanHost_Navig;
}

// 获取角度差值
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

// 清除标志位
void Roadway_Flag_clean(void)
{
    Stop_Flag = TRACKING;
    temp_MP = 0;
    Track_Mode = TrackMode_NONE;
    Moving_ByEncoder = ENCODER_NONE;
}

// 前后，转弯监测
void Moving_ByEncoderCheck(void)
{
    if (Moving_ByEncoder == ENCODER_NONE)
    {
        return;
    }

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
    case ENCODER_LEFT90:
        if (Mp_Value >= Turn_L90_MPval)
        {
            Stop();
            Stop_Flag = TURNCOMPLETE;
        }
        break;
    case ENCODER_LEFT45:
        if (Mp_Value >= Turn_L45_MPval)
        {
            Stop();
            Stop_Flag = TURNCOMPLETE;
        }
        break;
    case ENCODER_RIGHT90:
        if (Mp_Value >= Turn_R90_MPval)
        {
            Stop();
            Stop_Flag = TURNCOMPLETE;
        }
        break;
    case ENCODER_RIGHT45:
        if (Mp_Value >= Turn_R45_MPval)
        {
            Stop();
            Stop_Flag = TURNCOMPLETE;
        }
        break;
    case ENCODER_RIGHT180:
        if (Mp_Value >= Turn_MP180)
        {
            Stop();
            Stop_Flag = TURNCOMPLETE;
        }
        break;

    default:
        break;
    }
}

// 运动控制
void Roadway_Check(void)
{
    if (Track_Mode != TrackMode_NONE)
    {
        TRACK_LINE();
    }
    Moving_ByEncoderCheck();
    Control(LSpeed, RSpeed);
}

// 电机控制（速度区间 -100-5, 5-100)
void Control(int L_Speed, int R_Speed)
{
    if (L_Speed >= 0)
    {
        if (L_Speed > 100)
            L_Speed = 100;
        // if (L_Speed < 5)
        //     L_Speed = 5; //限制速度参数
    }
    else
    {
        if (L_Speed < -100)
            L_Speed = -100;
        // if (L_Speed > -5)
        //     L_Speed = -5; //限制速度参数
    }
    if (R_Speed >= 0)
    {
        if (R_Speed > 100)
            R_Speed = 100;
        // if (R_Speed < 5)
        //     R_Speed = 5; //限制速度参数
    }
    else
    {
        if (R_Speed < -100)
            R_Speed = -100;
        // if (R_Speed > -5)
        //     R_Speed = -5; //限制速度参数
    }

    LSpeed = L_Speed;
    RSpeed = R_Speed;

    Send_UpMotor(L_Speed, R_Speed);
}

void Get_Track(void)
{
    uint16_t tmp = Get_Host_UpTrack(TRACK_ALL);

    NumberOfWhite = 0;

    for (uint8_t i = 0; i < 7; i++)
    {
        Q7[i] = (tmp >> i) & 0x01;
        NumberOfWhite += Q7[i] ? 1 : 0;
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        H8[i] = (tmp >> (8 + i)) & 0x01;
        NumberOfWhite += H8[i] ? 1 : 0;
    }

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

// 计算方向权重
void Get_DirectionWights(void)
{
    static int PreviousWights = 0;

    int HWights = 0, QWeights = 0;

    for (uint8_t i = 1; i <= 4; i++)
    {
        HWights += H8[3 + i] - H8[4 - i];
    }
    for (uint8_t i = 1; i <= 4; i++)
    {
        QWeights += Q7[2 + i] - Q7[4 - i];
    }


    // for (uint8_t i = 1; i <= 4; i++)
    // {
    //     HWights += (H8[3 + i] * i) + (H8[4 - i] * (-i));
    // }
    // for (uint8_t i = 1; i <= 4; i++)
    // {
    //     QWeights += (Q7[2 + i] * i) + (Q7[4 - i] * (-i));
    // }
    DirectionWights = HWights + QWeights;

    DirectionWights = (DirectionWights + PreviousWights) >> 1; // 简单滤波
    PreviousWights = DirectionWights;

    Calculate_pid(DirectionWights);

#if _TRACK_OUTPUT_

    print_info("H:%d,Q:%d,T:%d\r\n", HWights, QWeights, DirectionWights);

#endif // _TRACK_OUTPUT_
}

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

    if ((NumberOfWhite >= ALL_WHITE) && (Track_Mode == TrackMode_NORMAL)) // 全白
    {
        // Stop();
        Stop_Flag = OUTTRACK;
    }
    else if ((NumberOfWhite <= ALL_BLACK) && (Track_Mode == TrackMode_NORMAL)) // 全黑
    {
        // Stop();
        Stop_Flag = CROSSROAD;
    }
    else
    {
        LSpeed = Car_Speed + PID_value;
        RSpeed = Car_Speed - PID_value;
        // Control(LSpeed, RSpeed);
    }
}

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
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        Mp_Value = Roadway_mp_Get();
        Roadway_Check(); //路况检测
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
}

//end file
