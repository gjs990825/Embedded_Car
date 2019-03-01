#include "stm32f4xx.h"
#include "CanP_Hostcom.h"
#include "delay.h"
#include "roadway_check.h"
#include "cba.h"
#include "tba.h"
#include "movement.h"
#include "debug.h"
#include "hardware.h"
// #include "mba.h"
// #include "pid.h"
// #include "includes.h"

#define _TRACK_OUTPUT_ 0

#define ALL_WHITE 14
#define ALL_BLACK 2



// Gao added
bool Q7[7] = {0};
bool H8[8] = {0};

uint8_t NumberOfWhite = 0;

int DirectionWights = 0;

// (Get_Host_UpTrack(TRACK_Q7)) & 0xff;

// 左右转向
uint8_t wheel_L_Flag = 0, wheel_R_Flag = 0;
// 转向角度
uint8_t Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0, Right180_Flag = 0;
// 转向
uint8_t wheel_Nav_Flag = 0;
// 前进后退
uint8_t Go_Flag = 0, Back_Flag = 0;
// 循迹
uint8_t Track_Flag = 0;
// 倒退循迹
uint8_t Track_Back_Flag = 0;
uint8_t Line_Flag = 0;

// 运行状态
uint8_t Stop_Flag = 0;
// 左右速度
int LSpeed = 0, RSpeed = 0;
// 设定车速
int Car_Spend = 0;

// 设定码盘值
uint16_t temp_MP = 0;
uint16_t temp_Nav = 0;

uint8_t Wheel_flag = 0;

// 循迹结果
u8 gd1, gd2;
// 调整车身
u8 h = 0;
// 循迹模式
u8 track_mode = 0;
// 获取的码盘值
uint16_t Mp_Value = 0;

void Track(uint8_t gd1, uint8_t gd2);
// 码盘中间变量
int16_t Roadway_cmp;
extern int16_t CanHost_Mp;

void Roadway_mp_syn(void) //码盘同步
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

//_______________________________________________________________

//_________________________________________________________
uint16_t Roadway_Navig;
extern uint16_t CanHost_Navig;

/*
	角度同步
**/
void Roadway_nav_syn(void)
{
    Roadway_Navig = CanHost_Navig;
}
/*
	获取角度差值
**/
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

//_______________________________________________________________

void Roadway_Flag_clean(void)
{
    wheel_L_Flag = 0;
    wheel_R_Flag = 0;
    Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0, Right180_Flag = 0;
    Go_Flag = 0;
    Back_Flag = 0;
    Track_Back_Flag = 0;
    Track_Flag = 0;
    Stop_Flag = 0;
    temp_MP = 0;
}

/**
	前进监测
*/
void Go_and_Back_Check(void)
{
    if (Go_Flag == 1)
    {
        if (temp_MP <= Roadway_mp_Get())
        {
            //			stop_Test();
            Go_Flag = 0;
            Stop_Flag = FORBACKCOMPLETE;
            Send_UpMotor(0, 0);
            // OSSemPost(&GOSEM, OS_OPT_POST_1, &err);
            //go_Test(38);
        }
    }
    else if (Back_Flag == 1)
    {
        if (temp_MP <= Roadway_mp_Get())
        {
            //			stop_Test();
            Back_Flag = 0;
            Stop_Flag = FORBACKCOMPLETE;
            Send_UpMotor(0, 0);
            // OSSemPost(&BACKSEM, OS_OPT_POST_1, &err);
            //back_Test(38);
        }
    }
    else if (Left45_Flag == 1)
    {
        if (Mp_Value >= Turn_L45_MPval)
        {

            stop_Test();
        }
    }
    else if (Left90_Flag == 1)
    {
        if (Mp_Value >= Turn_L90_MPval)
        {

            stop_Test();
        }
    }
    else if (Right45_Flag == 1)
    {
        if (Mp_Value >= Turn_R45_MPval)
        {

            stop_Test();
        }
    }
    else if (Right90_Flag == 1)
    {
        if (Mp_Value >= Turn_R90_MPval)
        {

            stop_Test();
        }
    }
    else if (Right180_Flag == 1)
    {
        if (Mp_Value >= Turn_MP180)
        {

            stop_Test();
        }
    }
}

uint8_t Roadway_GoBack_Check(void)
{
    return ((Go_Flag == 0) && (Back_Flag == 0) && (Track_Flag == 0) && (wheel_L_Flag == 0) && (wheel_R_Flag == 0)) ? 1 : 0;
}

/**
	码盘转弯
*/
void wheel_Nav_check(void)
{
    uint16_t Mp_Value = 0;

    if (wheel_Nav_Flag)
    {
        Mp_Value = Roadway_mp_Get();
        if (Mp_Value >= temp_Nav)
        {
            wheel_Nav_Flag = 0;
            Stop_Flag = TURNCOMPLETE;
            Send_UpMotor(0, 0);
        }
    }
}

/**
	根据循迹线转弯
*/
//uint32_t Mp_Value = 0;
void wheel_Track_check(void)
{
    uint16_t Track_Value = 0;

    if (wheel_L_Flag == 1)
    {
        Track_Value = Get_Host_UpTrack(TRACK_H8);
        if (!(Track_Value & 0X10)) //找到循迹线，停止
        {
            if (Wheel_flag > 50)
            {
                wheel_L_Flag = 0;
                Wheel_flag = 0;
                Stop_Flag = TURNCOMPLETE;
                Send_UpMotor(0, 0);
            }
        }
        else if (Track_Value == 0Xff)
        {
            Wheel_flag++;
        }
    }
    else if (wheel_R_Flag == 1)
    {
        Track_Value = Get_Host_UpTrack(TRACK_H8);

        if (!(Track_Value & 0X08)) //找到循迹线，停止
        {
            if (Wheel_flag > 50)
            {
                wheel_R_Flag = 0;
                Wheel_flag = 0;
                Stop_Flag = TURNCOMPLETE;
                Send_UpMotor(0, 0);
            }
        }
        else if (Track_Value == 0Xff)
        {
            Wheel_flag++;
        }
    }
}

/**
	循迹监测
*/
void Track_Check()
{

    if (Track_Flag == 1)
    {
        if (temp_MP < Mp_Value)
        {

            stop_Test();
        }

        gd1 = (Get_Host_UpTrack(TRACK_Q7)) & 0xff; // 获取循迹数据
        gd2 = (Get_Host_UpTrack(TRACK_H8)) & 0xff;

        if (gd1 < 0x40)
        {
            track_mode = 1;
        }

        else
            track_mode = 0;

        Track(gd1, gd2);
    }
}

void Roadway_Check(void)
{
    TRACK_LINE();
    // Track_Check();
    Go_and_Back_Check();
    //	wheel_mp_check();
    //	wheel_Nav_check();
}

/***************************************************************
** 功能：     电机控制函数
** 参数：	  L_Spend：电机左轮速度
**            R_Spend：电机右轮速度
** 返回值：   无	  
****************************************************************/
void Control(int L_Spend, int R_Spend)
{
    if (L_Spend >= 0)
    {
        if (L_Spend > 100)
            L_Spend = 100;
        if (L_Spend < 5)
            L_Spend = 5; //限制速度参数
    }
    else
    {
        if (L_Spend < -100)
            L_Spend = -100;
        if (L_Spend > -5)
            L_Spend = -5; //限制速度参数
    }
    if (R_Spend >= 0)
    {
        if (R_Spend > 100)
            R_Spend = 100;
        if (R_Spend < 5)
            R_Spend = 5; //限制速度参数
    }
    else
    {
        if (R_Spend < -100)
            R_Spend = -100;
        if (R_Spend > -5)
            R_Spend = -5; //限制速度参数
    }
    Send_UpMotor(L_Spend, R_Spend);
}

/***************************************************************
** 功能：     循迹函数
** 参数：	  无参数
** 返回值：   无
****************************************************************/
void Track(uint8_t gd1, uint8_t gd2)
{

    //	if( (gd1==0x3e)||(gd1==0x7e)||(gd1==0x3f)||(gd1==0x1f)||(gd1==0x7c) )     //十字路口检测到白卡  横放
    //	{
    //		if( (gd2!=0xfe) && (gd2!=0xfd) && (gd2!=0xfc) && (gd2!=0xff)&&(gd2!=0x7f) && (gd2!=0xbf) && (gd2!=0x3f) && (gd1!=0x7f) ){
    //		Track_Flag=0;
    //		Mp_Value=0;
    //		temp_MP=0;
    //		Send_UpMotor(0,0);
    //		// OSSemPost(&RFID_GOSEM,OS_OPT_POST_1,&err);
    //		}
    //	}
    //	if( (gd1==0x1c)||(gd1==0x1e)||(gd1==0x0e)||(gd1==0x0f)||(gd1==0x07)||(gd1==0x3c)||(gd1==0x38)||(gd1==0x78)||(gd1==0x70) )  //检测到竖着放的白卡
    //	{
    //		if( (gd2!=0x7f) && (gd2!=0xbf) && (gd2!=0x3f) && (gd2!=0xff)&&(gd2!=0xfe) && (gd2!=0xfd) && (gd2!=0xfc) && (gd1!=0x7f)){
    //		Track_Flag=0;
    //		Mp_Value=0;
    //		temp_MP=0;
    //		Send_UpMotor(0,0);
    //		// OSSemPost(&RFID_GOSEM,OS_OPT_POST_1,&err);
    //		}
    //	}

    if ((gd1 == 0x00) && (h == 0)) //小车位置调整
    {
        h = 1;
        Mp_Value = 0;
        temp_MP = 0;
        Track_MP(45);
    }
    else if ((gd2 == 0x00) && (h == 0))
    {
        h = 1;
        Track_MP(55);
    }
    else
    {

        if (track_mode == 0)
        {
#if PID_TRACK

            read_xj_values(gd1, gd2);
            calculate_pid();
#else
            Stop_Flag = TRACKING;
            if (gd2 == 0XE7 || gd1 == 0x77) //1、中间3/4传感器检测到黑线，全速运行
            {
                LSpeed = Car_Spend;
                RSpeed = Car_Spend;
            }
            if (Line_Flag != 2)
            {
                if (gd1 == 0x7B || gd1 == 0x73)
                {
                    LSpeed = Car_Spend + 20;
                    RSpeed = Car_Spend - 20;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x7B)
                {
                    LSpeed = Car_Spend + 10;
                    RSpeed = Car_Spend - 10;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x79 || gd1 == 0x7D)
                {
                    LSpeed = Car_Spend + 30;
                    RSpeed = Car_Spend - 40;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x7D)
                {
                    LSpeed = Car_Spend + 20;
                    RSpeed = Car_Spend - 30;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x7C || gd1 == 0x7E)
                {
                    LSpeed = Car_Spend + 40;
                    RSpeed = Car_Spend - 60;
                    Line_Flag = 1;
                }
                else if (gd1 == 0x7E)
                {

                    LSpeed = Car_Spend + 30;
                    RSpeed = Car_Spend - 40;
                    Line_Flag = 1;
                }
                if (gd2 == 0xf7)
                {
                    LSpeed = LSpeed + 10;
                    RSpeed = RSpeed - 20;
                    Line_Flag = 0;
                }
                else if (gd2 == 0XF3 || gd2 == 0XFB) //2、中间4、3传感器检测到黑线，微右拐

                {
                    LSpeed = LSpeed + 30;
                    RSpeed = RSpeed - 30;
                    Line_Flag = 0;
                }
                else if (gd2 == 0XF9 || gd2 == 0XFD) //3、中间3、2传感器检测到黑线，再微右拐

                {
                    LSpeed = LSpeed + 40;
                    RSpeed = RSpeed - 60;
                    Line_Flag = 0;
                }
                else if (gd2 == 0XFC) //4、中间2、1传感器检测到黑线，强右拐
                {
                    LSpeed = LSpeed + 50;
                    RSpeed = RSpeed - 90;
                    Line_Flag = 0;
                }
                else if (gd2 == 0XFE) //5、最右边1传感器检测到黑线，再强右拐

                {
                    LSpeed = LSpeed + 60;
                    RSpeed = RSpeed - 120;
                    Line_Flag = 1;
                }
            }

            if (Line_Flag != 1)
            {

                if (gd1 == 0x6F || gd1 == 0x67)
                {
                    RSpeed = Car_Spend + 20;
                    LSpeed = Car_Spend - 20;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x6F)
                {
                    LSpeed = Car_Spend + 10;
                    RSpeed = Car_Spend - 10;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x4F || gd1 == 0x5F)
                {
                    RSpeed = Car_Spend + 30;
                    LSpeed = Car_Spend - 40;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x5F)
                {
                    RSpeed = Car_Spend + 20;
                    LSpeed = Car_Spend - 30;
                    Line_Flag = 0;
                }
                else if (gd1 == 0x3F || gd1 == 0x1F)
                {
                    RSpeed = Car_Spend + 40;
                    LSpeed = Car_Spend - 60;
                    Line_Flag = 2;
                }
                else if (gd1 == 0x3F)
                {
                    RSpeed = Car_Spend + 30;
                    LSpeed = Car_Spend - 40;
                    Line_Flag = 2;
                }
                if (gd2 == 0XCF) //6、中间6、5传感器检测到黑线，微左拐

                {
                    RSpeed = RSpeed + 30;
                    LSpeed = LSpeed - 30;
                    Line_Flag = 0;
                }
                else if (gd2 == 0X9F || gd2 == 0XDF) //7、中间7、6传感器检测到黑线，再微左拐

                {
                    RSpeed = RSpeed + 40;
                    LSpeed = LSpeed - 60;
                    Line_Flag = 0;
                }
                else if (gd2 == 0X3F || gd2 == 0XBF) //8、中间8、7传感器检测到黑线，强左拐
                {
                    RSpeed = RSpeed + 50;
                    LSpeed = LSpeed - 90;
                    Line_Flag = 0;
                }
                else if (gd2 == 0X7F) //9、最左8传感器检测到黑线，再强左拐

                {
                    RSpeed = RSpeed + 60;
                    LSpeed = LSpeed - 120;
                    Line_Flag = 2;
                }
            }

            if (gd2 == 0X3F || gd2 == 0XBF) //8、中间8、7传感器检测到黑线，强左拐
            {
                RSpeed = RSpeed + 50;
                LSpeed = LSpeed - 90;
                Line_Flag = 0;
            }
            else if (gd2 == 0X7F) //9、最左8传感器检测到黑线，再强左拐

            {
                RSpeed = RSpeed + 60;
                LSpeed = LSpeed - 120;
                Line_Flag = 2;
            }
#endif
        }
        //        else if (track_mode == 1)
        //        {
        //            read_xj_values2(gd1);

        ////            calculate_pid();
        //        }

        if (gd1 == 0xFF || gd2 == 0xFF)
        { //循迹灯全亮
        }

        if (Track_Flag != 0)
        {
#if PID_TRACK
            motor_control();
#else
            Control(LSpeed, RSpeed);
#endif
        }
        //	else
        //	{
        //		Send_UpMotor(0,0);
        //	}
    }
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

void Get_DirectionWights(void)
{
    static int PreviousWights = 0;

    int HWights = 0, QWeights = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        HWights += (H8[4 + i] * i * i) + (H8[3 - i] * i * (-i));
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        QWeights += (Q7[3 + i] * i * i) + (Q7[3 - i] * i * (-i));
    }
    DirectionWights = HWights + QWeights;

    DirectionWights = (DirectionWights + PreviousWights) / 2; // 滤波
    PreviousWights = DirectionWights;

#if _TRACK_OUTPUT_

    print_info("H:%d,Q:%d,T:%d\r\n", HWights, QWeights, DirectionWights);

#endif // _TRACK_OUTPUT_
}

void TRACK_LINE(void)
{
    if (!Track_Flag) // 不用循迹，跳出
    {
        return;
    }

    Get_Track();
    Get_DirectionWights();

    if ((DirectionWights == 0) && (NumberOfWhite >= ALL_WHITE)) // 全白
    {
        // Track_MP(55);
        Stop_Flag = OUTTRACK;
    }
    else if ((DirectionWights == 0) && (NumberOfWhite <= ALL_BLACK)) // 全黑
    {
        Stop_Flag = CROSSROAD;
        
    }
    else
    {
        LSpeed = Car_Spend + DirectionWights * 5;
        RSpeed = Car_Spend - DirectionWights * 5;
        Control(LSpeed, RSpeed);
    }
}

void roadway_check_TimInit(uint16_t arr, uint16_t psc)
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
    //    OSIntEnter();
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        Mp_Value = Roadway_mp_Get();
        Roadway_Check(); //路况检测
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
    //    OSIntExit();
}

//end file
