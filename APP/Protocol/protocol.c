#include "protocol.h"
#include "uart_a72.h"

// �������ݵ����ڣ�A72�����壩
void Send_DataToUsart(uint8_t *buf, uint32_t length)
{
    UartA72_TxClear();
    UartA72_TxAddStr(buf, length);
    UartA72_TxStart();
}
