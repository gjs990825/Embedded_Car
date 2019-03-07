#include "protocol.h"
#include "uart_a72.h"

#if 0
// 函数实现需要固定的函数类型(如无返回值无参的void(Task *)(void))
// C语言中实现较为复杂，所以使用宏定义
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
