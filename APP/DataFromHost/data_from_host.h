#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]
// #define Host_AGV_Return_Flag CommandFlagStatus[FromHost_AGVReturn].Data[0]
// #define StreetFlag CommandFlagStatus[FromHost_StreetLight].Data[0]

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t cmd);

extern ZigBee_DataStatus_t ETC_Status, BarrierGate_Status;

#endif // _DATA_FROM_HOST_H_
