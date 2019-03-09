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

void TrafficLight_Task(void)
{
    Send_ZigBeeData(ZigBee_TrafficLightStartRecognition, 2, 200); // 开始识别交通灯
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlag(GetCmdFlag(FromHost_TrafficLight), SET); // 等待识别完成
}

#define Ultrasonic_Task(times) Ultrasonic_GetAverage(times)

// TFT
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition); // 请求识别TFT内容
    WaitForFlag(GetCmdFlag(FromHost_TFTRecognition), SET); // 等待识别完成
    Request_ToHost(RequestCmd_TFTShow); //请求显示车牌到TFT
}

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

uint8_t HEXTOBCD(uint8_t hex_data)
{
    uint8_t temp;
    temp = (hex_data / 10 * 16 + hex_data % 10);
    return temp;
}

void LEDDispaly_ShowDistance(uint8_t dis)
{
    Infrared_LEDDisplayDistanceData[4] = HEXTOBCD(dis / 100);
    Infrared_LEDDisplayDistanceData[5] = HEXTOBCD(dis % 100);
    Send_ZigbeeData_To_Fifo(Infrared_LEDDisplayDistanceData, 8);
}

void Task_5_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(135), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);
    Beep(2);
    delay_ms(600);
    delay_ms(600); // 等待摄像头反应

    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
    Beep(5);
    delay_ms(500);

    RotationLED_Task();

    ExcuteAndWait(Turn_ByEncoder(45), Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT; // 与任务开始时候方向不一致
}

void Task_3_5(void)
{
    ExcuteAndWait(Turn_ByEncoder(20), Stop_Flag, TURNCOMPLETE);

    TrafficLight_Task();

    ExcuteAndWait(Turn_ByEncoder(-20), Stop_Flag, TURNCOMPLETE);
}

void Task_1_5(void)
{
    QRCode_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 14), Stop_Flag, FORBACKCOMPLETE);

    LEDDispaly_ShowDistance(Ultrasonic_Task(10));

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 14), Stop_Flag, FORBACKCOMPLETE);
}

void Task_1_3(void)
{
    // 路灯
}

void Task_5_3(void)
{
    // （语音）
}

void Task_5_1(void)
{
    // ETC
}

void Task_3_1(void)
{
    // 入库
}
