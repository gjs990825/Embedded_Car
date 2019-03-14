#include "task.h"
#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "syn7318.h"
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

// 交通灯识别
void TrafficLight_Task(void)
{
    Send_ZigBeeData(ZigBee_TrafficLightStartRecognition, 2, 200); // 开始识别交通灯
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlag(GetCmdFlag(FromHost_TrafficLight), SET); // 等待识别完成
}

// TFT图形图像识别
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition);             // 请求识别TFT内容
    WaitForFlag(GetCmdFlag(FromHost_TFTRecognition), SET); // 等待识别完成
    Request_ToHost(RequestCmd_TFTShow);                    //请求显示车牌到TFT
}

// 旋转led发送车牌
void RotationLED_Task(void)
{
    Infrared_Send_A(Infrared_PlateData1);
    delay_ms(600);
    Infrared_Send_A(Infrared_PlateData2);
}

// 二维码识别
void QRCode_Task(void)
{
    Request_ToHost(RequestCmd_QRCode1);
    WaitForFlag(GetCmdFlag(FromHost_QRCodeRecognition), SET);
}

// 起始任务
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

// 终止任务
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

// led显示距离（输入距离）
void LEDDispaly_ShowDistance(uint16_t dis)
{
    ZigBee_LEDDisplayDistanceData[4] = HEX2BCD(dis / 100);
    ZigBee_LEDDisplayDistanceData[5] = HEX2BCD(dis % 100);
    Check_Sum(ZigBee_LEDDisplayDistanceData);
    Send_ZigbeeData_To_Fifo(ZigBee_LEDDisplayDistanceData, 8);
}

// 路灯档位调节，输入目标档位自动调整
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

    bubble_sort(temp_val, 4); // 对获得数据排序算出初始档位
    for (i = 0; i < 4; i++)
    {
        if (CurrentLightValue == temp_val[i])
        {
            errorValue = (int8_t)targetLevel - (i + 1);
            break;
        }
    }

    if (errorValue >= 0) // 调整到目标档位
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

// ETC任务
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // 摇摆10次，不开直接走
    {
        if ((ETC_Status.isSet == SET) && Check_IsTimeOut(ETC_Status.timeStamp, 6 * 1000)) // 六秒前的数据作废
            break;
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE); // 跟着节拍
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE); // 一起摇摆
    }
}

// 下面是坐标点对应的任务集合，独立任务进入前需要保证位置距离朝向等准确无误
// 任务结束和开始车身方向不一样的需要手动设置 CurrentStaus.dir = DIR_XX;

void Task_5_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(90 + 50), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 10), Stop_Flag, FORBACKCOMPLETE);
    Beep(2);
    delay_ms(700);
    delay_ms(700); // 等待摄像头反应

    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 10), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-50 - 35), Stop_Flag, TURNCOMPLETE);
    Beep(5);
    delay_ms(500);

    RotationLED_Task();

    ExcuteAndWait(Turn_ByEncoder(35), Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT; // 与任务开始时方向不一致
}

void Task_3_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(22), Stop_Flag, TURNCOMPLETE);
    delay_ms(700);

    TrafficLight_Task();

    ExcuteAndWait(Turn_ByEncoder(-22), Stop_Flag, TURNCOMPLETE);
}

void Task_1_5(void)
{
    delay_ms(700);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

    QRCode_Task();
    LEDDispaly_ShowDistance(Ultrasonic_GetAverage(20));

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
}

void Task_1_3(void)
{
    // 路灯// Request_ToHost(RequestCmd_StreetLight);
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

    StreetLight_Task(3);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
}

void Task_5_3(void)
{
    // 语音暂缓
    // SYN7318_Test();
    ExcuteAndWait(Turn_ByEncoder(90 + 45), Stop_Flag, TURNCOMPLETE);

    Infrared_Send_A(Infrared_AlarmON);

    ExcuteAndWait(Turn_ByEncoder(-45), Stop_Flag, TURNCOMPLETE);

    Send_ZigbeeData_To_Fifo(ZigBee_AGVStart, 8);

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
    // 入库
    delay_ms(500);
    Send_ZigbeeData_To_Fifo(ZigBee_WirelessChargingON, 8);
    delay_ms(700);
    End_Task();
}
