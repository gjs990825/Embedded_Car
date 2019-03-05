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

#define ExcuteNTimes(task, N)       \
    for (uint8_t i = 0; i < N; i++) \
    {                               \
        task;                       \
    }

void LED_Timer(bool status)
{
    Send_ZigbeeData_To_Fifo(status ? SMG_JSK : SMG_JSG, 8);
}

void LED_TimerStart(void)
{
    ExcuteNTimes(Send_ZigbeeData_To_Fifo(SMG_JSK, 8); delay_ms(20), 3);
}

void LED_TimerStop(void)
{
    ExcuteNTimes(Send_ZigbeeData_To_Fifo(SMG_JSG, 8); delay_ms(20), 3);
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
