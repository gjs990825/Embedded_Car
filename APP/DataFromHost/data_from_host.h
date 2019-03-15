#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t cmd);

extern ZigBee_DataStatus_t ETC_Status, BarrierGate_Status;

#endif // _DATA_FROM_HOST_H_
