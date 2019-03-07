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

extern uint8_t QR_OK;
extern uint8_t CP_AND_SHAPE;

// TFT
void TFT_Task(void)
{
    ExcuteAndWait(Turn_Right90, Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Turn_Right45, Stop_Flag, TURNCOMPLETE);

    Beep(2);

    TFTPage_Next();
    Request_ToHost(Request_PlateRecognition);
    // WaitForFlag(CP_AND_SHAPE, 1);

    Beep(3);
    delay_ms(500);

    TFTPage_Next();
    Request_ToHost(Request_ShapeRecongnition);
    // WaitForFlag(CP_AND_SHAPE, 2);

    Beep(4);
    delay_ms(500);

    TFTPage_Next();
    Request_ToHost(Request_TFTShow);

    Beep(5);
    delay_ms(500);

    ExcuteAndWait(Turn_Left45, Stop_Flag, TURNCOMPLETE);
    CurrentStaus.dir = DIR_LEFT;
}

// 二维码识别（未完成）
void QRCode_Task(void)
{
    Request_ToHost(Request_QRCode1);
    WaitForFlag(QR_OK, 1);
    ////////
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
