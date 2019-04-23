#include "protocol.h"

// 请求数据的buffer
uint8_t Data_PlateNumber[DataLength_PlateNumber];
uint8_t Data_QRCode1[DataLength_QRCode1];
uint8_t Data_QRCode2[DataLength_QRCode2];
uint8_t Data_QRCodeSecondCar[DataLength_QRCodeSecondCar];
uint8_t Data_TrafficLight[DataLength_TrafficLight];
uint8_t Data_ShapeNumber[DataLength_ShapeNumber];
uint8_t Data_ColorNumber[DataLength_ColorNumber];
uint8_t Data_ShapeColorNumber[DataLength_ShapeColorNumber];
uint8_t Data_RFID[DataLength_RFID];
uint8_t Data_Preset1[DataLength_Preset1];
uint8_t Data_Preset2[DataLength_Preset2];
uint8_t Data_Preset3[DataLength_Preset3];

// 储存buffer指针/长度/状态的结构体数组
DataSetting_t DataBuffer[] = {
    {(uint8_t *)NULL, 0, 0},
    {Data_PlateNumber, DataLength_PlateNumber, RESET},
    {Data_QRCode1, DataLength_QRCode1, RESET},
    {Data_QRCode2, DataLength_QRCode2, RESET},
    {Data_QRCodeSecondCar, DataLength_QRCodeSecondCar, RESET},
    {Data_TrafficLight, DataLength_TrafficLight, RESET},
    {Data_ShapeNumber, DataLength_ShapeNumber, RESET},
    {Data_ColorNumber, DataLength_ColorNumber, RESET},
    {Data_ShapeColorNumber, DataLength_ShapeColorNumber, RESET},
    {Data_RFID, DataLength_RFID, RESET},
    {Data_Preset1, DataLength_Preset1, RESET},
    {Data_Preset2, DataLength_Preset2, RESET},
    {Data_Preset3, DataLength_Preset3, RESET},
};

// 请求指令使用的buffer
uint8_t Request_ToHostArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
uint8_t ZigBee_LEDDisplayDataToSecondRow[8] = {0x55, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0xBB};
uint8_t ZigBee_AGVStart[8] = {0x55, 0x02, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xBB};
uint8_t ZigBee_AGVPreset[8] = {0x55, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00, 0xBB};

// 红外发送车牌数据
uint8_t Infrared_PlateData1[6] = {0xFF, 0x20, 0x41, 0x31, 0x42, 0x32};
uint8_t Infrared_PlateData2[6] = {0xFF, 0x10, 0x43, 0x35, 0x45, 0x31};

//报警台数据
uint8_t Infrared_AlarmData[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};

// 红外数据
uint8_t Infrared_Data[6];

// 未处理/未使用/未知 的 指令/数据
// u8 BJM1[8] = {0x55, 0x02, 0xd1, 0x00, 0x00, 0x00, 0x00, 0xbb}; //发送给从车的报警码
// u8 BJM2[8] = {0x55, 0x02, 0xd2, 0x00, 0x00, 0x00, 0x00, 0xbb};
// u8 HW_BJ[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};
// u8 DW[8] = {0x55, 0x02, 0xd3, 0x00, 0x00, 0x00, 0x00, 0xbb};
// u8 AGV_start[8] = {0x55, 0x02, 0xd0, 0x00, 0x00, 0x00, 0x00, 0xBB};
// u8 AGV_ZNLD[8] = {0x55, 0x02, 0xd3, 0x00, 0x00, 0x00, 0x00, 0xBB}; //预案路灯档位设置

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

#if !USE_MACRO_FUNCTIONS

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
