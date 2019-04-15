#include "debug.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "independent_task.h"

// #define varNumber 2
// #define var1 Go_Flag
// #define var2 Stop_Flag

// uint8_t *watch[5] = {&var1, &var2};
// uint8_t var_tmp[5] = {0};

void print_info(char *str, ...)
{
    uint16_t len;
    uint8_t buf[50];
    va_list ap;
    va_start(ap, str);
    vsprintf((char *)buf, str, ap);
    va_end(ap);
    len = strlen((char *)buf);
    Send_InfoData_To_Fifo(buf, len);
}

void DebugTimer_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM_InitStructure.TIM_Period = arr;
    TIM_InitStructure.TIM_Prescaler = psc;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &TIM_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM5, DISABLE);
}

// 初始化调试用引脚
void DebugPin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    DEBUG_PIN_1_RESET();
    DEBUG_PIN_2_RESET();
    DEBUG_PIN_3_RESET();
}

// void Debug_CheckVar(void)
// {
//     for (uint8_t i = 0; i < varNumber; i++)
//     {
//         if (*(int *)watch[i] != var_tmp[i])
//         {
//             print_info("V%d>%d->%d\r\n", i, var_tmp[i], *watch[i]);
//             var_tmp[i] = *watch[i];
//         }
//     }
// }


void TIM5_IRQHandler(void)
{    
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
    {
        if (FOUND_RFID_CARD && RFID_RoadSection)
        {
            TIM_Cmd(TIM5, DISABLE);
            RFID_Task();
            Resume_StatusBeforeFoundRFID(0); // 前后距离差值更正，恢复运动状态
        }
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
}
