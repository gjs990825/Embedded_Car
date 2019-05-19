#include "roadway_check.h"
#include "delay.h"
#include "cba.h"
#include "tba.h"
#include "movement.h"
#include "debug.h"
#include "pid.h"
#include "my_lib.h"
#include "canp_hostcom.h"
#include "independent_task.h"

#define _ENABLE_TURNING_BY_TRACK_ 1

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
        preSpeedChanging = millis(); // 更新时间戳
    }
    else
    {
        // 间隔一定时间后发送一次速度信息
        // 防止数据丢失造成匀速行驶时的严重错误
        if (IsTimeOut(preSpeedChanging, 150))
        {
            Send_UpMotor(LSpeed, RSpeed);
            preSpeedChanging = millis();
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

// 计算偏差值
float Get_Offset(void)
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

#if _TRACK_OUTPUT_

    static uint32_t t = 0;

    if (t++ == 5)
    {
        print_info("ERR %2.2f\r\n", errorValue);
        t = 0;
    }

#endif // _TRACK_OUTPUT_

    return errorValue;
}

// 循迹
void TRACK_LINE(void)
{
    static float offset = 0;

    if (Track_Mode == TrackMode_ENCODER)
    {
        if (temp_MP <= Mp_Value)
        {
            Stop_WithoutPIDClear();
            Stop_Flag = FORBACKCOMPLETE;
            return;
        }
    }

    // 没有接收到循迹信息不进行运算
    // 循迹板20ms上传一次循迹信息
    if (Get_TrackInfoReceived())
    {
        Get_Track();
        offset = Get_Offset();
        Calculate_pid(offset);
    }

#if _ENABLE_TURNING_BY_TRACK_

    if (Track_Mode == TrackMode_Turn) // 通过循迹线转向
    {
        extern uint8_t turnLeftOrRight;
        static uint8_t TrackStatus = 0;
        static uint32_t outTrackStamp;

        if (TrackStatus == 0) // 未出线
        {
            if (IS_All_WHITE())
            {
                TrackStatus = 1;
                outTrackStamp = millis();
            }
        }
        else if (TrackStatus == 1) // 出线缓冲+未寻到黑线
        {
            if (!IS_All_WHITE() && IsTimeOut(outTrackStamp, 70))
            {
                TrackStatus = 2;
                PidData_Clear();
                PidData_Set(offset, (turnLeftOrRight == DIR_RIGHT) ? 85 : -85);
            }
        }
        else if (TrackStatus == 2) // 转到循迹线
        {
            // 使用PID循迹
            Update_MotorSpeed(PID_value, -PID_value);

            if (turnLeftOrRight == DIR_RIGHT) //右转
            {
                if (offset < 1.0f)
                {
                    TrackStatus = 0;
                    Stop();
                    Stop_Flag = TURNCOMPLETE;
                }
            }
            else // 左转
            {
                if (offset > -1.0f)
                {
                    TrackStatus = 0;
                    Stop();
                    Stop_Flag = TURNCOMPLETE;
                }
            }
        }
        return;
    }

#endif // _ENABLE_TURNING_BY_TRACK_
    
    // 判断循迹灯并分类处理
    // 循迹灯全白
    if (IS_All_WHITE()) 
    {
        if ((Track_Mode == TrackMode_NORMAL) || (Track_Mode == TrackMode_ENCODER)) // 循迹状态
        {
            // 白卡路段
            if (RFID_RoadSection)
            {
                FOUND_RFID_CARD = true; // 找到白卡
                Save_RunningStatus();   // 保存运行状态
                Stop();                 // 暂停运行
                TIM_Cmd(TIM5, ENABLE);  // 开启定时器
            }
            // 特殊地形路段
            else if (Special_RoadSection)
            {
                ENTER_SPECIAL_ROAD = true; // 进入特殊地形
                Save_RunningStatus();      // 保存运行状态
                Stop();                    // 暂停运行
                TIM_Cmd(TIM5, ENABLE);     // 开启定时器
            }
            else
            {
                // 全白不停止运行
                // Stop();
                Stop_Flag = OUTTRACK; // 出线
                Update_MotorSpeed(Car_Speed, Car_Speed);
            }
        }
    }
    // 全黑
    else if (IS_ALL_BLACK()) 
    {
        if (Track_Mode == TrackMode_NORMAL)
        {
            Roadway_Flag_clean();
            PidData_Clear(); // 遇黑线清空PID数据
            Update_MotorSpeed(0, 0);
            Stop_Flag = CROSSROAD;
        }
        else if (Track_Mode == TrackMode_ENCODER)
        {
            Stop();
            Stop_Flag = FORBACKCOMPLETE;
        }
    }
    // 其它
    else
    {
        // 判定循迹灯是否有反白的情况，反白为十字路口的白卡时情况
        // 中间三个传感器必须检测到白色，两边两个至少有一个黑色，且下一坐标为十字路口
        if (((H8[0] + Q7[0] + H8[7] + Q7[6]) <= 3) && (H8[3] & H8[4] & Q7[3]) && (((NextStatus.x % 2) && (NextStatus.y % 2)) != 0))
        {
            // 白卡路段
            if (RFID_RoadSection)
            {
                // 处理白卡
                if ((FOUND_RFID_CARD == false))
                {
                    Roadway_Flag_clean();    // 清空标志位
                    Update_MotorSpeed(0, 0); // 停下
                    Stop_Flag = CROSSROAD;   // 标记为十字路口
                    FOUND_RFID_CARD = true;  // 找到白卡
                    Save_RunningStatus();    // 保存当前状态
                    Stop();                  // 暂停运行
                    TIM_Cmd(TIM5, ENABLE);   // 使能RFID处理定时器
                }
                else
                {
                    // 已处理，等待定时器溢出中断
                }
            }
            // 非白卡路段
            else
            {
                // 标记十字路口并停车
                Roadway_Flag_clean();
                PidData_Clear();
                Update_MotorSpeed(0, 0);
                Stop_Flag = CROSSROAD;
            }
        }
        // 非反白，常规循迹状态
        else
        {
            // 结合PID计算结果控制速度
            Update_MotorSpeed(Car_Speed + PID_value, Car_Speed - PID_value);
        }
    }
    // CAN发送出现过没有送达的现象，速度控制在定时器中断内实现
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
        if (IsTimeOut(lastStopStamp, _STOP_WAITING_INTERVAL_))
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
