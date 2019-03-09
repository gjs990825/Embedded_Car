#if !defined(_DATA_FROM_HOST_H_)
#define _DATA_FROM_HOST_H_

#include "sys.h"

#define autoRunEnable CommandFlagStatus[FromHost_Start]
// #define Host_AGV_Return_Flag CommandFlagStatus[FromHost_AGVReturn].Data[0]
// #define StreetFlag CommandFlagStatus[FromHost_StreetLight].Data[0]

void Process_DataFromHost(uint8_t mainCmd);


#endif // _DATA_FROM_HOST_H_
