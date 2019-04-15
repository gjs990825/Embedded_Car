#include "roadway_check.h"
#include "delay.h"
#include "cba.h"
#include "tba.h"
#include "movement.h"
#include "debug.h"
#include "pid.h"
#include "my_lib.h"
#include "stdlib.h"
#include "canp_hostcom.h"
#include "independent_task.h"

// 转向是否完成
#define CheckTurnComplete(EncoderValue) \
    if (Mp_Value >= EncoderValue)       \
    {                                   \
        Stop();                         \
        Stop_Flag = TURNCOMPLETE;       \
    }

// 前七个循迹传感器
int8_t Q7[7] = {0};
// 后八个循迹传感器
int8_t H8[8] = {0};
// 所有循迹传感器数据按左到右顺序排列
int8_t ALL_TRACK[15];
// 当前白色数量
uint8_t NumberOfWhite = 0;

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
    switch (Moving_ByEncoder)
    {
    case ENCODER_GO:
        if (temp_MP <= Mp_Value)
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
        }
        break;
    case ENCODER_BACK:
        if (temp_MP <= Mp_Value)
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
    if (!Get_TrackInfoReceived())
        return;
    else
    {
        Set_TrackInfoReceived(false);
    }

    uint16_t tmp = Get_Host_UpTrack(TRACK_ALL);
    // 清空循迹灯亮起个数
    NumberOfWhite = 0;

    // 获取循迹灯信息和循迹灯亮灯数量
    for (uint8_t i = 0; i < 7; i++)
    {
        Q7[i] = (tmp >> i) & 0x01;
        H8[i] = (tmp >> (8 + i)) & 0x01;

        NumberOfWhite += Q7[i] ? 1 : 0;
        NumberOfWhite += H8[i] ? 1 : 0;

        ALL_TRACK[i * 2] = H8[i];
        ALL_TRACK[i * 2 + 1] = Q7[i];
    }
    H8[7] = (tmp >> (15)) & 0x01;
    NumberOfWhite += H8[7] ? 1 : 0;
    ALL_TRACK[14] = H8[7];

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

// 计算输出速度值
void Calculate_Speed(void)
{
    // 计算各个点与临近点的和
    int8_t all_weights[15] = {0};
    for (uint8_t i = 1; i < 14; i++)
    {
        for (uint8_t j = i - 1; j <= i + 1; j++)
        {
            all_weights[i] += ALL_TRACK[j];
        }
    }

    // 遍历找到最小值
    int8_t minimum = 3;
    for (uint8_t i = 1; i < 14; i++)
    {
        if (minimum > all_weights[i])
            minimum = all_weights[i];
    }

    // 获取最小值正续和倒序编号（两个最小值情况）
    // 丢弃第一位和最后一位，没有数据
    int8_t errorValue1 = 0, errorValue2 = 14;
    for (;;)
    {
        if (all_weights[++errorValue1] == minimum)
            break;
    }
    for (;;)
    {
        if (all_weights[--errorValue2] == minimum)
            break;
    }

    // 平均之后计算误差值
    float errorValue = 7.0 - (errorValue1 + errorValue2) / 2.0;

// 滤波
#define FILTER_ARRAY_LENGTH 3
    static float filterArray[FILTER_ARRAY_LENGTH] = {0};
    static uint8_t currentNumber = 0;

    filterArray[currentNumber++] = errorValue;
    if (currentNumber >= FILTER_ARRAY_LENGTH)
        currentNumber = 0;

    errorValue = 0;
    for (uint8_t i = 0; i < FILTER_ARRAY_LENGTH; i++)
    {
        errorValue += filterArray[i];
    }
    errorValue /= (float)FILTER_ARRAY_LENGTH;

    Calculate_pid(errorValue);

#if _TRACK_OUTPUT_

    static uint32_t t = 0;

    if (t++ == 5)
    {
        print_info("ERR %2.2f\r\n", errorValue);
        t = 0;
    }

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
        if (temp_MP <= Mp_Value)
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
            return;
        }
    }

    // 没有接收到循迹信息不进行运算
    if (Get_TrackInfoReceived())
    {
        Get_Track();
        Calculate_Speed();
    }

#if _ENABLE_TURNING_BY_TRACK_

    if (Track_Mode == TrackMode_Turn) // 通过循迹线转向（效果不是很好，基本不用）
    {
        if (TrackStatus == 0)
        {
            if (IS_All_WHITE(NumberOfWhite))
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

#endif // _ENABLE_TURNING_BY_TRACK_

    if (IS_All_WHITE(NumberOfWhite)) // 全白
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
        if (RFID_RoadSection) // 白卡路段
        {
            // 判定循迹灯是否有反白的情况 // 最中间三位必须为白色，最两边至少有一个黑色，并且下一坐标为路口
            if (((H8[0] + Q7[0] + H8[7] + Q7[6]) <= 3) && (H8[3] & H8[4] & Q7[3]) && (((NextStatus.x % 2) && (NextStatus.y % 2)) != 0))
            {
                if (FOUND_RFID_CARD == false) //
                {
                    Roadway_Flag_clean(); // 处理遇到十字线的情况
                    Update_MotorSpeed(0, 0);
                    Stop_Flag = CROSSROAD;
                    FOUND_RFID_CARD = true;       // 找到白卡
                    Save_StatusBeforeFoundRFID(); // 保存当前状态
                    Stop();                       // 暂停运行
                    TIM_Cmd(TIM5, ENABLE);        // 使能RFID处理定时器
                }
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

void TIM1_BRK_TIM9_IRQHandler(void)
{
    extern uint32_t lastStopStamp;

    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        // DEBUG_PIN_2_SET();

        // 上一次停止时间未等待足够时间则不进行下一个动作，防止打滑
        if (IsTimeOut(lastStopStamp, 300))
        {
            DEBUG_PIN_2_SET();

            Mp_Value = Roadway_mp_Get();
            Roadway_Check();

            DEBUG_PIN_2_RESET();
        }

        // DEBUG_PIN_2_RESET();
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
}
