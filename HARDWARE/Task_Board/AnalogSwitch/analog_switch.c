#include "analog_switch.h"

#define Switch_COM PBout(15)
#define Switch_A PHout(10)
#define Switch_B PHout(11)
#define Switch_C PAout(15)

void AnalogSwitch_PortInit(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOH, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    
    // PB15
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // PA15
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // PH10\11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    // 输出0到通道7（空）
    AnalogSwitch_Output(0);
    AnalogSwitch_CelectChannel(7);
}

// 选择通道
void AnalogSwitch_CelectChannel(uint8_t channel)
{
    if (channel >= 8)
        return;

    Switch_A = ((channel & 0x01) == 0x01);
    Switch_B = ((channel & 0x02) == 0x02);
    Switch_C = ((channel & 0x04) == 0x04);
}

// 在当前通道输出数据
void AnalogSwitch_Output(bool status)
{
    Switch_COM = status;
}
