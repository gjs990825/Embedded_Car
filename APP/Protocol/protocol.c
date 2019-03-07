#include "protocol.h"
#include "uart_a72.h"

#if 0
// ����ʵ����Ҫ�̶��ĺ�������(���޷���ֵ�޲ε�void(Task *)(void))
// C������ʵ�ֽ�Ϊ���ӣ�����ʹ�ú궨��
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

// �������ݵ�����(A72������)
void Send_DataToUsart(uint8_t *buf, uint32_t length)
{
    UartA72_TxClear();
    UartA72_TxAddStr(buf, length);
    UartA72_TxStart();
}
