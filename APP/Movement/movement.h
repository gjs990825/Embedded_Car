#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"

void Auto_Run(void);


void Go_ToNextNode(Route_Task_t next);
void Start_Tracking(int speed);	

void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_Left45(void);
void Turn_Left90(void);
void Turn_Right45(void);
void Turn_Right90(void);
void Turn_Right180(void);

void Track_ByEncoder(int speed, uint16_t setMP);


void Auto_RouteTask(RouteNode current, uint8_t taskN);

#endif // _MOVEMENT_H_
