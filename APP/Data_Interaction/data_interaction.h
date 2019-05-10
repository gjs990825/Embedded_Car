#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]
extern bool AGV_MissonComplete;

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
DeclareExternDataStatus(TrafficLight);
DeclareExternDataStatus(StereoGarage);
DeclareExternDataStatus(AGV);
DeclareExternDataStatus(VoiceBroadcast);

// 指令、数据处理

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t *cmd);
void HostData_Handler(uint8_t *buf);

// 标志物状态获取

bool Get_BarrierGateStatus(void);
uint8_t Get_StereoGrageLayer(void);
uint8_t *Get_StereoGrageInfraredStatus(void);

// 向从车获取数据

int8_t Get_AGVQRCode(uint8_t **data);
uint16_t Get_AGVUltrasonic(void);
uint16_t Get_AGVBrightness(void);

// 向上位机请求任务

void RequestToHost_Task(uint8_t request);

// 向上位机发送数据

void Send_QRCodeData(uint8_t *QRData, uint8_t length);
void Send_RFIDData(uint8_t *RFIDData, uint8_t length);

// 向上位机请求数据

uint8_t *Get_PlateNumber(void);
uint8_t *Get_QRCode(uint8_t QRID, uint8_t use);
uint8_t Get_TrafficLight(void);
uint8_t Get_ShapeNumber(uint8_t Shape);
uint8_t Get_ColorNumber(uint8_t Color);
uint8_t Get_ShapeColorNumber(uint8_t Shape, uint8_t Color);
uint8_t *Get_RFIDInfo(uint8_t *data);
uint8_t *Get_ShapeInfo(void);
uint8_t Get_AllColorCount(void);

#endif // _DATA_FROM_HOST_H_
