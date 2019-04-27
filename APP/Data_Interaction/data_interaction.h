#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]

// 获取返回状态
#define Get_ZigBeeReturnStatus(name) (name##_Status.isSet == SET)
// 获取返回数据
#define Get_ZigBeeReturnData(name) name##_Status.cmd
// 获取返回时间戳
#define Get_ZigBeeReturnStamp(name) (name##_Status.timeStamp)

#define DeclareExternDataStatus(name) extern ZigBee_DataStatus_t name##_Status

DeclareExternDataStatus(BarrierGate);
DeclareExternDataStatus(ETC);
DeclareExternDataStatus(AGVComplete);
DeclareExternDataStatus(TrafficLight);
DeclareExternDataStatus(StereoGarage);
DeclareExternDataStatus(AGV);
DeclareExternDataStatus(VoiceBroadcast);

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t *cmd);
void HostData_Handler(uint8_t *buf);

void HostData_RequestSingle(uint8_t requestID);
void HostData_RequestMulti(uint8_t requestID, uint8_t *param, uint8_t paramLen);

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
