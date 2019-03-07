#include "protocol.h"
#include "uart_a72.h"

// 请求指令使用的buffer
uint8_t Request_ToHostArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
uint8_t ZigBee_LEDDisplayData[8];
uint8_t ZigBee_LEDDisplayDistance[8];
uint8_t ZigBee_AGVStart[8];

#if 0
// C中没有泛型，有些函数不容易实现，所以这里宏定义实现
void ExcuteNTimes(void(Task *)(void), N, delay)
{
    for (uint8_t i = 0; i < N; i++)
    {
        Task();
        delay_ms(delay);
    }
}
#endif

#if !USE_MACRO_FUNCTIONS

void Send_ZigBeeData(uint8_t *data, uint8_t ntimes, uint16_t delay)
{
    ExcuteNTimes(Send_ZigbeeData_To_Fifo(data, 8), ntimes, delay);
}

void Request_ToHost(uint8_t request)
{
    Request_ToHostArray[Pack_Ending] = request;
    Send_ToHost(Request_ToHostArray, 8);
}

#endif

// 发送数据到串口(A72开发板)
void Send_DataToUsart(uint8_t *buf, uint32_t length)
{
    UartA72_TxClear();
    UartA72_TxAddStr(buf, length);
    UartA72_TxStart();
}
