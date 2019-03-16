#include "voice.h"
#include "delay.h"
#include <string.h>
#include "cba.h"
#include "infrared.h"
#include "data_base.h"
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"

// #define SYN7318_RST_H GPIO_SetBits(GPIOB, GPIO_PinSource9)
// #define SYN7318_RST_L GPIO_ResetBits(GPIOB, GPIO_PinSource9)

uint8_t USART6_RX_BUF[USART6_RX_LEN] = {0};
uint8_t USART6_TX_BUF[USART6_TX_LEN] = {0};
uint16_t USART6_RX_STA = 0;

// unsigned char Wake_Up[] = {0xfd, 0x00, 0x02, 0x51, 0x1F};
// unsigned char Stop_Wake_Up[] = {0xFD, 0x00, 0x01, 0x52};

// unsigned char Start_ASR_Buf[] = {0xFD, 0x00, 0x02, 0x10, 0x03};
// unsigned char Stop_ASR_Buf[] = {0xFD, 0x00, 0x01, 0x11};

// unsigned char Play_MP3[] = {0xFD, 0x00, 0x1E, 0x01, 0x01, 0xC6, 0xF4, 0xB6, 0xAF, 0xD3, 0xEF, 0xD2, 0xF4,
//                             0xBF, 0xD8, 0xD6, 0xC6, 0xBC, 0xDD, 0xCA, 0xBB, 0xA3, 0xAC, 0xC7, 0xEB,
//                             0xB7, 0xA2, 0xB3, 0xF6, 0xD6, 0xB8, 0xC1, 0xEE};

void USART6_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_TypeDefStructure;
    USART_InitTypeDef USART_TypeDefStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

    //PC6-Tx
    GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_AF;   //复用功能
    GPIO_TypeDefStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
    GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_TypeDefStructure);

    USART_TypeDefStructure.USART_BaudRate = baudrate;                                  //波特率
    USART_TypeDefStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件控制流
    USART_TypeDefStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 //接收与发送模式
    USART_TypeDefStructure.USART_Parity = USART_Parity_No;                             //无校验位
    USART_TypeDefStructure.USART_StopBits = USART_StopBits_1;                          //停止位1
    USART_TypeDefStructure.USART_WordLength = USART_WordLength_8b;                     //数据位8位
    USART_Init(USART6, &USART_TypeDefStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART6, ENABLE);
    USART_ClearFlag(USART6, USART_FLAG_TC);   //清除发送完成标志位
    USART_ClearFlag(USART6, USART_FLAG_RXNE); //清除接收完成标志位
}

void USART6_SendChar(uint8_t ch)
{
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
        ;
    USART_SendData(USART6, ch);
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
        ;
}

void USART6_print(char *str, ...)
{
    uint16_t len;
    va_list ap;
    va_start(ap, str);
    vsprintf((char *)USART6_TX_BUF, str, ap);
    va_end(ap);
    len = strlen((char *)USART6_TX_BUF);

    for (uint16_t i = 0; i < len; i++)
    {
        USART6_SendChar(USART6_TX_BUF[i]);
    }
}


// uint16_t USART6_RX_STA = 0; 0000 0000 0000 0000
void USART6_IRQHandler(void)
{
    static uint16_t cmdLenth = 0;
    if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET)
    {
        uint8_t ch = USART_ReceiveData(USART6);

        if (ch == 0xFC) // 遇到FC重新接收
        {
            USART6_RX_STA = 0x8000;
			cmdLenth = 0;
        }
		else if (USART6_RX_STA == 0x8000) // 第一位长度
        {
            cmdLenth = ch;
            USART6_RX_STA = 0xC000;
        }
        else if (USART6_RX_STA == 0xC000) // 第二位长度
        {
            cmdLenth <<= 8;
            cmdLenth |= ch;
            USART6_RX_STA = 0xE000;
        }
        else if (USART6_RX_STA == 0xE000)
        {
            if ((USART6_RX_STA & 0x0FFF) < cmdLenth) // 小于指令长度
            {
                USART6_RX_BUF[USART6_RX_STA & 0x0FFF] = ch;
				USART6_RX_STA++;
                if (USART6_RX_STA >= cmdLenth)
                {
                    USART6_RX_STA |= 0xF000;
                    for(uint8_t i = 0; i < cmdLenth; i++)
                    {
                        print_info("%X", USART6_RX_BUF[i]);
                    }
                    print_info("\r\n");
					USART6_RX_STA = 0;
                }
            }
        }
    }
    USART_ClearITPendingBit(USART6, USART_IT_RXNE);
}
