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

#define CheckTurnComplete(EncoderValue) \
    if (Mp_Value >= EncoderValue)       \
    {                                   \
        Stop();                         \
        Stop_Flag = TURNCOMPLETE;       \
    }

// 使能循迹输出
#define _TRACK_OUTPUT_ 0

// 超过此数判定出线
#define ALL_WHITE 15
// 低于此数判定撞线
#define ALL_BLACK 7

// 前七个循迹传感器
int8_t Q7[7] = {0};
// 后八个循迹传感器
int8_t H8[8] = {0};
// 白色数量
uint8_t NumberOfWhite = 0;
// 方向权重(PID适应线性值，这里已更改为偏移值)
int DirectionWights = 0;
// 运行状态
uint8_t Stop_Flag = TRACKING;
// 循迹模式
uint8_t Track_Mode = TrackMode_NONE;
// 定值前后和转向
Moving_ByEncoder_t Moving_ByEncoder = ENCODER_NONE;
// 定角度转向目标码盘值
uint16_t TurnByEncoder_Value = 0;

// 左右速度
int LSpeed = 0, RSpeed = 0;
// 循迹时车速
int Car_Speed = 0;

// 目标码盘值
uint16_t temp_MP = 0;
// 当前的码盘值
uint16_t Mp_Value = 0;

// 之前获取的码盘值
int16_t Roadway_cmp;
// 从CAN获取的实时码盘值
extern int16_t CanHost_Mp;

// 码盘同步
void Roadway_mp_syn(void)
{
    Roadway_cmp = CanHost_Mp;
}

// 码盘获取
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
    case ENCODER_TurnByValue:
        CheckTurnComplete(TurnByEncoder_Value);
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

// 电机控制（速度区间 -100 ~ 100)
void Control(int L_Speed, int R_Speed)
{
    LSpeed = constrain_int(L_Speed, -100, 100);
    RSpeed = constrain_int(R_Speed, -100, 100);
    Send_UpMotor(LSpeed, RSpeed);
}

// 获取循迹信息，计算白色的数量
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

// 计算方向权重
void Get_DirectionWights(void)
{
    static int PreviousWights = 0;

    DirectionWights = 0;

    for (uint8_t i = 1; i <= 4; i++)
    {
        DirectionWights += (H8[3 + i] - H8[4 - i]);
    }
    for (uint8_t i = 1; i <= 4; i++)
    {
        DirectionWights += (Q7[2 + i] - Q7[4 - i]);
    }

    DirectionWights = (DirectionWights + PreviousWights) >> 1; // 简单滤波
    PreviousWights = DirectionWights;

    Calculate_pid(DirectionWights);

#if _TRACK_OUTPUT_

    print_info("T:%d\r\n", DirectionWights);

#endif // _TRACK_OUTPUT_
}

// 循迹
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

    if (NumberOfWhite >= ALL_WHITE) // 全白
    {
        if ((Track_Mode == TrackMode_NORMAL) || (Track_Mode == TrackMode_ENCODER)) // 循迹状态
        {
            if (RFID_RoadSection) // 白卡路段
            {
                FOUND_RFID_CARD = true;       // 找到白卡
                Save_StatusBeforeFoundRFID(); // 保存当前状态
                Stop();                       // 暂停运行
                TIM_Cmd(TIM5, ENABLE);        // 使能RFID处理定时器
            }
            else
            {
                // Stop();
                Stop_Flag = OUTTRACK; // 出线
            }
        }
    }
    else if ((NumberOfWhite <= ALL_BLACK) && (Track_Mode == TrackMode_NORMAL)) // 全黑
    {
        Roadway_Flag_clean();
        Control(0, 0);
        Stop_Flag = CROSSROAD;
    }
    else
    {
        LSpeed = Car_Speed + PID_value;
        RSpeed = Car_Speed - PID_value;
        // Control(LSpeed, RSpeed);
        // 因CAN发送出现过没有送达的现象，速度控制在定时器中断内实现
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
