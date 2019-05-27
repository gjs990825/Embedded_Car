#include "seven_segment_display.h"
#include "analog_switch.h"

#define SerialInput PCout(13) // SER
#define LatchClock PFout(11)  // RCK
#define ShiftClock PGout(8)   // SCK

// 控制数码管A
#define Select_A(x)                            \
    AnalogSwitch_CelectChannel(Channel_SMG_A); \
    AnalogSwitch_Output(x);

// 控制数码管B
#define Select_B(x)                            \
    AnalogSwitch_CelectChannel(Channel_SMG_B); \
    AnalogSwitch_Output(x);

// 显示数据
const uint8_t SevenSegmentDisplayCode[] = {
    0xc0, 0xf9, 0xa4, 0xb0,
    0x99, 0x92, 0x82, 0xf8,
    0x80, 0x90, 0x88, 0xc6,
    0x86, 0x8e, 0x89, 0xc7};

// 数码管端口初始化
void SevenSegmentDisplay_PortInit(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    // PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PF11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    // PG8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    ShiftClock = 0;
    SerialInput = 0;
    LatchClock = 0;
    SevenSegmentDisplay_HEX(0x00);
    SevenSegmentDisplay_HEX(0x00);
}

// // 写入数据并输出
// void HC595_Write_Data(uint8_t data)
// {
//     for (uint8_t i = 0; i < 8; i++)
//     {
//         ShiftClock = 0;

//         SerialInput = ((data & 0x80) == 0x80);
//         data <<= 1;

//         ShiftClock = 1;
//     }
//     LatchClock = 0;
//     LatchClock = 1;
// }

// 写入数据并输出
// MSB First
void HC595_Write_Data(uint8_t data)
{
    LatchClock = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        SerialInput = !!(data & (1 << (7 - i)));
        ShiftClock = 1;
        ShiftClock = 0;
    }
    LatchClock = 1;
}

// 十六进制数据显示
// 一次显示一半
void SevenSegmentDisplay_HEX(uint8_t hexData)
{
    static bool A_OR_B = true;

    uint8_t data;

    if (A_OR_B)
    {
        data = (hexData >> 4) & 0x0F;
        Select_B(1); // 关闭B
        Select_A(1); // 打开A
    }
    else
    {
        data = hexData & 0x0F;
        Select_A(0); // 关闭A
        Select_B(0); // 打开B
    }
    HC595_Write_Data(SevenSegmentDisplayCode[data]);
}

// void HC595_Write_Data(uint8_t dis_data)
// {
//     for (uint8_t i = 0; i < 8; i++)
//     {
//         SCK_RESET();
//         if ((dis_data & 0x80) == 0x80)
//         {
//             DSP_SET();
//         }
//         else
//         {
//             DSP_RESET();
//         }
//         dis_data = dis_data << 1;
//         SCK_SET();
//     }
//     RCK_RESET();
//     RCK_SET();
// }

// #define DSP_SET() GPIO_SetBits(GPIOB, GPIO_Pin_15)
// #define DSP_RESET() GPIO_ResetBits(GPIOB, GPIO_Pin_15)

// #define SCK_SET() GPIO_SetBits(GPIOH, GPIO_Pin_11)
// #define SCK_RESET() GPIO_ResetBits(GPIOH, GPIO_Pin_11)

// #define RCK_SET() GPIO_SetBits(GPIOH, GPIO_Pin_10)
// #define RCK_RESET() GPIO_ResetBits(GPIOH, GPIO_Pin_10)

// void display(uint8_t a, uint8_t b)
// {
//     while (1)
//     {
//         GPIO_ResetBits(GPIOC, GPIO_Pin_13);
//         HC595_Write_Data(SevenSegmentDisplayCode[a]);
//         delay_ms(5);
//         GPIO_SetBits(GPIOC, GPIO_Pin_13);
//         HC595_Write_Data(SevenSegmentDisplayCode[b]);
//         delay_ms(5);
//     }
// }
