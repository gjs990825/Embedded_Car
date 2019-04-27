#include "protocol.h"
#include "my_lib.h"

// 请求ID和数据长度在一块的版本，暂未使用
#define GetRequestID(request) (DataRequest_##request >> 4)
#define GetRequestLength(request) (DataRequest_##request & 0x08)

// 定义数据buffer
#define DefineBuffer(X) uint8_t Data_##X[DataLength_##X]
// 在结构体中给出buffer指针和长度
#define DataAndLength(X)                \
    {                                   \
        Data_##X, DataLength_##X, RESET \
    }

// 请求数据的buffer
DefineBuffer(PlateNumber);
DefineBuffer(QRCode1);
DefineBuffer(QRCode2);
DefineBuffer(QRCodeSecondCar);
DefineBuffer(TrafficLight);
DefineBuffer(ShapeNumber);
DefineBuffer(ColorNumber);
DefineBuffer(ShapeColorNumber);
DefineBuffer(RFID);
DefineBuffer(Preset1);
DefineBuffer(Preset2);
DefineBuffer(Preset3);

// 储存buffer指针/长度/状态的结构体数组
DataSetting_t DataBuffer[] = {
    {(uint8_t *)NULL, 0, 0},
    DataAndLength(PlateNumber),
    DataAndLength(QRCode1),
    DataAndLength(QRCode2),
    DataAndLength(QRCodeSecondCar),
    DataAndLength(TrafficLight),
    DataAndLength(ShapeNumber),
    DataAndLength(ColorNumber),
    DataAndLength(ShapeColorNumber),
    DataAndLength(RFID),
    DataAndLength(Preset1),
    DataAndLength(Preset2),
    DataAndLength(Preset3),
};

uint8_t DATA_REQUEST_NUMBER = GET_ARRAY_LENGEH(DataBuffer);

// 请求指令使用的buffer
uint8_t Request_ToHostArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

uint8_t CommandFlagStatus[0xFF] = {0};

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


// 单次发送，带校验
void Send_ZigBeeData(uint8_t *data)
{
    Check_Sum(data);
    Send_ZigbeeData_To_Fifo(data, 8);
}

// 多次发送，带校验
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay)
{
    Check_Sum(data);
    for (uint8_t i = 0; i < ntimes; i++)
    {
        Send_ZigbeeData_To_Fifo(data, 8);
        if (delay > 0)
        {
            delay_ms(delay);
        }
    }
}

// 向上位机请求任务
void Request_ToHost(uint8_t request)
{
    Request_ToHostArray[Pack_MainCmd] = request;
    Send_ToHost(Request_ToHostArray, 8);
}

// 向上位机请求数据
void Request_Data(uint8_t dataRequest[2])
{
    Request_ToHostArray[Pack_MainCmd] = dataRequest[0];
    Request_ToHostArray[Pack_SubCmd1] = dataRequest[1];
    Send_ToHost(Request_ToHostArray, 8);
}

// 发送数据到串口(A72开发板)
void Send_DataToUsart(uint8_t *buf, uint32_t length)
{
    UartA72_TxClear();
    UartA72_TxAddStr(buf, length);
    UartA72_TxStart();
}

// 将校验和填入cmd[Pack_CheckSum]中
void Check_Sum(uint8_t *cmd)
{
    uint16_t temp = cmd[2] + cmd[3] + cmd[4] + cmd[5];
    cmd[Pack_CheckSum] = (uint8_t)(temp % 256);
}
