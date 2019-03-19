#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]

extern ZigBee_DataStatus_t ETC_Status, BarrierGate_Status, AGVComplete_Status;

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t cmd);
void HostData_Handler(uint8_t *buf);


#endif // _DATA_FROM_HOST_H_
