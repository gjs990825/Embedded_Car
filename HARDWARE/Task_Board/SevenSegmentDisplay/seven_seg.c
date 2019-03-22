#include "seven_seg.h"
#include "hardware.h"
#include "delay.h"
#define DSP_SET GPIO_SetBits(GPIOB, GPIO_Pin_15)
#define DSP_RESET GPIO_ResetBits(GPIOB, GPIO_Pin_15)
#define SCK_SET GPIO_SetBits(GPIOH, GPIO_Pin_11)
#define SCK_RESET GPIO_ResetBits(GPIOH, GPIO_Pin_11)
#define RCK_SET GPIO_SetBits(GPIOH, GPIO_Pin_10)
#define RCK_RESET GPIO_ResetBits(GPIOH, GPIO_Pin_10)

static uint8_t SMG[] = {
    0xc0, 0xf9, 0xa4, 0xb0,
    0x99, 0x92, 0x82, 0xf8,
    0x80, 0x90, 0x88, 0xc6,
    0x86, 0x8e, 0x89, 0xc7};

void HC595_Write_Data(uint8_t dis_data)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        SCK_RESET;
        if ((dis_data & 0x80) == 0x80)
        {
            DSP_SET;
        }
        else
        {
            DSP_RESET;
        }
        dis_data = dis_data << 1;
        SCK_SET;
    }
    RCK_RESET;
    RCK_SET;
}

// ÏÔÊ¾²âÊÔ£¨ÓÐËÀÑ­»·£©
void display(uint8_t a, uint8_t b)
{
    while (1)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        HC595_Write_Data(SMG[a]);
        delay_ms(5);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        HC595_Write_Data(SMG[b]);
        delay_ms(5);
    }
}
