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

extern uint8_t QR_OK;
extern uint8_t CP_AND_SHAPE;

// TFT
void TFT_Task(void)
{
    ExcuteAndWait(Turn_Right90, Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Turn_Right45, Stop_Flag, TURNCOMPLETE);

    Beep(2);

    ExcuteNTimes(Send_ZigbeeData_To_Fifo(TFTPage_Next, 8), 2);
    FillRequestAndSend(Request_PlateRecognition);
    WaitForFlag(CP_AND_SHAPE, 1);

    print_info("Plate:");
    for (uint8_t i = 3; i <= 5; i++)
    {
        print_info("%x ", Wifi_Rx_Buf[i]);
    }
    print_info("\r\n");

    Beep(3);

    ExcuteNTimes(Send_ZigbeeData_To_Fifo(TFTPage_Next, 8), 2);
    FillRequestAndSend(Request_ShapeRecongnition);
    WaitForFlag(CP_AND_SHAPE, 2);

    print_info("Shape:");
    for (uint8_t i = 3; i <= 5; i++)
    {
        print_info("%x ", Wifi_Rx_Buf[i]);
    }
    print_info("\r\n");

    Beep(4);

    ExcuteNTimes(Send_ZigbeeData_To_Fifo(TFTPage_Next, 8), 2);
    FillRequestAndSend(Request_TFTShow);

    Beep(5);

    delay_ms(500);
    ExcuteAndWait(Turn_Left45, Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT;
}

// 未完成
void QRCode_Task(void)
{
    FillRequestAndSend(Request_QRCode1);
    WaitForFlag(QR_OK, 1);

    LED_TimerStop();
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

void LED_Timer(bool status)
{
    Send_ZigbeeData_To_Fifo(status ? LED_Display_StartTimer : LED_Display_StopTimer, 8);
}

void LED_TimerStart(void)
{
    ExcuteNTimes(LED_Timer(SET); delay_ms(20), 3);
}

void LED_TimerStop(void)
{
    ExcuteNTimes(LED_Timer(RESET); delay_ms(20), 3);
}

void Test_Task_1(void)
{
    Set_tba_WheelLED(L_LED, SET);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
}

void Test_Task_2(void)
{
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    Set_tba_WheelLED(R_LED, RESET);
}

void Test_Task_3(void)
{
    Set_tba_WheelLED(L_LED, SET);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    Set_tba_WheelLED(R_LED, RESET);
}
