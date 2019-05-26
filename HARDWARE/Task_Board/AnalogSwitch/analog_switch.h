#if !defined(_ANALOG_SWITCH_H_)
#define _ANALOG_SWITCH_H_

#include "sys.h"

// 通道定义
enum
{
    Channel_LED_R, // LED_R
    Channel_LED_L, // LED_L
    Channel_SMG_A, // 数码管A
    Channel_SMG_B, // 数码管B
    Channel_INC,   // 超声波发射
    Channel_IR,    // 红外发射
    Channel_BEEP,  // 蜂鸣器
    Channel_NONE,  // 未使用
};

void AnalogSwitch_PortInit(void);
void AnalogSwitch_CelectChannel(uint8_t channel);
void AnalogSwitch_Output(bool status);

#endif // _ANALOG_SWITCH_H_
