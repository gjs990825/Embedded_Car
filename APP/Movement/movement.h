#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"

enum 
{
	DIR_CLOCKWISE = 0,
	DIR_COUNTCLOCKWISE
};

typedef enum
{
	TRACKING = 0x00,		// 循迹状态
	CROSSROAD = 0x01,		// 十字路口
	TURNCOMPLETE = 0x02,	// 转弯完成
	FORBACKCOMPLETE = 0x03, // 前进后退完成(定长循迹)
	OUTTRACK = 0x04			// 出线
} StopFlag_t;

#define WaitForFlag(flag, status) \
	do                            \
	{                             \
		while (flag != status)    \
		{                         \
		};                        \
	} while (0)

#define ExcuteAndWait(action, Flag, waitStatus) \
	do                                          \
	{                                           \
		action;                               \
		WaitForFlag(Flag, waitStatus);          \
		Stop();                                 \
	} while (0)

void Auto_Run(void);

void Go_ToNextNode(Route_Task_t next);
void Start_Tracking(int speed);

void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_ByEncoder(int16_t digree);

void Track_ByEncoder(int speed, uint16_t setMP);

void Auto_RouteTask(RouteNode current, uint8_t taskN);

#endif // _MOVEMENT_H_
