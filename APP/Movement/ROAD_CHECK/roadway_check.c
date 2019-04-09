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

// 转向是否完成
#define CheckTurnComplete(EncoderValue) \
    if (Mp_Value >= EncoderValue)       \
    {                                   \
        Stop();                         \
        Stop_Flag = TURNCOMPLETE;       \
    }

// 使能循迹信息输出
#define _TRACK_OUTPUT_ 0

// 超过此数判定出线/遇到白卡
#define ALL_WHITE 15
// 低于此数判定撞线
#define ALL_BLACK 7

// 前七个循迹传感器
int8_t Q7[7] = {0};
// 后八个循迹传感器
int8_t H8[8] = {0};
// 当前白色数量
uint8_t NumberOfWhite = 0;
// 方向权重(PID适应线性值，这里已更改为偏移值)
int DirectionWights = 0;
// 运行状态
uint8_t Stop_Flag = TRACKING;
// 循迹模式
TrackMode_t Track_Mode = TrackMode_NONE;
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

// 更新速度值（速度区间 -100 ~ 100)
void Update_MotorSpeed(int L_Speed, int R_Speed)
{
    // 速度限幅
    LSpeed = constrain_int(L_Speed, -100, 100);
    RSpeed = constrain_int(R_Speed, -100, 100);
}

// 电机控制，提交电机速度更改
void Submit_SpeedChanges(void)
{
    static int preLSpeed, preRSpeed;  // 上次的速度数据
    static uint32_t preSpeedChanging; // 上次变更速度的时间戳

    // 速度值改变则上传数据
    if (LSpeed != preLSpeed || RSpeed != preRSpeed)
    {
        preLSpeed = LSpeed;
        preRSpeed = RSpeed;
        Send_UpMotor(LSpeed, RSpeed);
        preSpeedChanging = Get_GlobalTimeStamp(); // 更新时间戳
    }
    else
    {
        // 间隔一定时间后发送一次速度信息，防止can丢包造成匀速行驶时的严重错误
        if (IsTimeOut(preSpeedChanging, 150))
        {
            Send_UpMotor(LSpeed, RSpeed);
            preSpeedChanging = Get_GlobalTimeStamp();
        }
    }
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

    if (Track_Mode == TrackMode_Turn) // 通过循迹线转向（效果不是很好，基本不用）
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

    if (NumberOfWhite >= ALL_WHITE) // 全白
    {
        if ((Track_Mode == TrackMode_NORMAL) || (Track_Mode == TrackMode_ENCODER)) // 循迹状态
        {
            if (isOutTrack == false)
            {
                isOutTrack = true;
                outTrackStamp = Get_GlobalTimeStamp();
            }
            else
            {
                if (IsTimeOut(outTrackStamp, 60)) // 遇全白超过60ms
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
        }
    }
    else if ((NumberOfWhite <= ALL_BLACK) && (Track_Mode == TrackMode_NORMAL)) // 全黑
    {
        Roadway_Flag_clean();
        Update_MotorSpeed(0, 0);
        Stop_Flag = CROSSROAD;
        isOutTrack = false;
    }
    else
    {
        if (RFID_RoadSection && (FOUND_RFID_CARD == false)) // 白卡路段，第一次遇到卡
        {
            // 判定循迹灯是否有反白的情况 // 最中间三位必须为白色，最两边至少有一个黑色，并且下一坐标为路口
            if (((H8[0] + Q7[0] + H8[7] + Q7[6]) <= 3) && (H8[3] & H8[4] & Q7[3]) && (((NextStatus.x % 2) && (NextStatus.y % 2)) != 0))
            {
                Roadway_Flag_clean(); // 处理遇到十字线的情况
                Update_MotorSpeed(0, 0);
                Stop_Flag = CROSSROAD;
                FOUND_RFID_CARD = true;       // 找到白卡
                Save_StatusBeforeFoundRFID(); // 保存当前状态
                Stop();                       // 暂停运行
                TIM_Cmd(TIM5, ENABLE);        // 使能RFID处理定时器
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
            // 因CAN发送出现过没有送达的现象，速度控制在定时器中断内实现
        }
    }
}

// 运动控制
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

// 路况检测（TIM9）
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

        // 上一次停止时间未等待足够时间则不进行下一个动作，防止打滑
        if (IsTimeOut(lastStopStamp, 300))
        {
            Mp_Value = Roadway_mp_Get();
            Roadway_Check();
        }
        // DEBUG_PIN_2_RESET();
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
}
