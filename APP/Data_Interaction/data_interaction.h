#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

// 自动运行开
#define autoRunEnable CommandFlagStatus[FromHost_Start]
// AGV任务完成
extern bool AGV_MissionComplete;

// 获取ZigBee返回状态
#define Get_ZigBeeReturnStatus(name) (name##_Status.isSet == SET)
// 复位ZigBee返回状态
#define Reset_ZigBeeReturnStatus(name) name##_Status.isSet = RESET
// 获取ZigBee返回数据
#define Get_ZigBeeReturnData(name) name##_Status.cmd
// 获取ZigBee返回时间戳
#define Get_ZigBeeReturnStamp(name) (name##_Status.timeStamp)

#define WaitZigBeeFlag(name, timeout) WaitForFlagInMs(name##_Status.isSet, SET, timeout)

// 声明变量
#define DeclareExternDataStatus(name) extern ZigBee_DataStatus_t name##_Status

DeclareExternDataStatus(BarrierGate);
DeclareExternDataStatus(ETC);
DeclareExternDataStatus(TrafficLight_A);
DeclareExternDataStatus(TrafficLight_B);
DeclareExternDataStatus(StereoGarage_A);
DeclareExternDataStatus(StereoGarage_B);
DeclareExternDataStatus(AGV);
DeclareExternDataStatus(VoiceBroadcast);

// 指令、数据处理

void Process_CommandFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t *cmd);
void HostData_Handler(uint8_t *buf);

// 标志物状态获取

bool Get_BarrierGateStatus(void);
uint8_t Get_StereoGrageLayer(uint8_t garage_x);
uint8_t *Get_StereoGrageInfraredStatus(uint8_t garage_x);

// 向从车获取数据

int8_t Get_AGVQRCode(uint8_t **data);
uint16_t Get_AGVUltrasonic(void);
uint16_t Get_AGVBrightness(void);

// 向上位机请求任务

void RequestToHost_Task(uint8_t request);

// 向上位机发送数据

void Send_QRCodeData(uint8_t *QRData, uint8_t length);
void Send_RFIDData(uint8_t RFIDx, uint8_t *RFIDData, uint8_t length);

void Send_PresetData(uint8_t preset_x, uint8_t *data, uint8_t length);

// 向上位机请求数据

uint8_t *Get_PlateNumber(uint8_t TFTx);
uint8_t *Get_QRCode(uint8_t QRCode_x, uint8_t use);
uint8_t Get_TrafficLight(uint8_t light_x);
uint8_t Get_ShapeNumber(uint8_t TFTx, uint8_t Shape);
uint8_t Get_ColorNumber(uint8_t TFTx, uint8_t Color);
uint8_t Get_ShapeColorNumber(uint8_t TFTx, uint8_t Shape, uint8_t Color);
uint8_t *Get_RFIDInfo(uint8_t RFIDx);
uint8_t *Get_TFTInfo(uint8_t TFTx);
uint8_t Get_AllColorCount(uint8_t TFTx);
uint8_t Get_AllShapeCount(uint8_t TFTx);

uint8_t *Get_PresetData(uint8_t preset_x);

#endif // _DATA_FROM_HOST_H_
