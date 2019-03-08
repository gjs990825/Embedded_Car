#include "protocol.h"

// ����ָ��ʹ�õ�buffer
uint8_t Request_ToHostArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
uint8_t ZigBee_LEDDisplayData[8];
uint8_t ZigBee_LEDDisplayDistance[8];
uint8_t ZigBee_AGVStart[8];

#if 0
// C��û�з��ͣ���Щ����������ʵ�֣���������궨��ʵ��
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
    Check_Sum(data);
    ExcuteNTimes(Send_ZigbeeData_To_Fifo(data, 8), ntimes, delay);
}

void Request_ToHost(uint8_t request)
{
    Request_ToHostArray[Pack_MainCmd] = request;
    Send_ToHost(Request_ToHostArray, 8);
}

void Request_Data(uint8_t dataRequest[2])
{
    Request_ToHostArray[Pack_MainCmd] = dataRequest[0];
    Request_ToHostArray[Pack_SubCmd1] = dataRequest[1];
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

// ��У�������cmd[Pack_CheckSum]��
void Check_Sum(uint8_t *cmd)
{
    uint8_t temp = cmd[2] + cmd[3] + cmd[4] + cmd[5];
    cmd[Pack_CheckSum] = (temp <= 0xFF) ? temp : (temp % 256);
}
