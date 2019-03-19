#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"
#include "protocol.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]

extern ZigBee_DataStatus_t ETC_Status, BarrierGate_Status, AGVComplete_Status;

void Process_DataFromHost(uint8_t mainCmd);
void ZigBee_CmdHandler(uint8_t cmd);
void HostData_Handler(uint8_t *buf);

void HostData_RequestSingle(uint8_t requestID);
void HostData_RequestMulti(uint8_t requestID, uint8_t *param, uint8_t paramLen);
uint8_t *Get_PlateNumber(void);


#endif // _DATA_FROM_HOST_H_
