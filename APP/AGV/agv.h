#if !defined(_AGV_H_)
#define _AGV_H_

#include "sys.h"
#include "route.h"

void Send_AGVCmd(uint8_t *cmd);

void AGV_Stop(void);
void AGV_Go(uint8_t speed, uint16_t encoder);
void AGV_Back(uint8_t speed, uint16_t encoder);
void AGV_Turn(int16_t speed);
void AGV_TrackLine(uint8_t speed);
void AGV_ClearEncoder(void);
void AGV_SendInfraredData(uint8_t irData[6]);
void AGV_TurnningLightControl(bool left, bool right);
void AGV_Beep(bool status);
void AGV_PhotoChange(bool dir);
void AGV_LightAdd(uint8_t level);
void AGV_UploadData(bool sta);
void AGV_VoiceRecognition(bool sta);

void AGV_Start(void);
void AGV_SetTowards(uint8_t towards);
void AGV_SetRouteFromTask(RouteNode_t task[], uint8_t length);
void AGV_SetRoute(uint8_t *str);
void AGV_SetTaskID(uint8_t routeNumber, uint8_t taskNumber);

#endif // _AGV_H_
