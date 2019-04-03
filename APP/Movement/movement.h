#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"
#include "Timer.h"

// Stop_Flag的枚举
typedef enum
{
	TRACKING = 0x00,		// 循迹状态
	CROSSROAD = 0x01,		// 十字路口
	TURNCOMPLETE = 0x02,	// 转弯完成
	FORBACKCOMPLETE = 0x03, // 前进后退完成(定长循迹)
	OUTTRACK = 0x04			// 出线
} StopFlag_t;

// 等待某个标志位。注意：此指令没有等待超时处理机制
#define WaitForFlag(flag, status) \
	do                            \
	{                             \
		while (flag != status)    \
		{                         \
		};                        \
	} while (0)

// 等待某个标志位，可设定超时时间
#define WaitForFlagInMs(flag, status, timeout)                                       \
	do                                                                               \
	{                                                                                \
		uint32_t startStamp = Get_GlobalTimeStamp();                                 \
		while ((Get_GlobalTimeStamp() < (startStamp + timeout)) && (flag != status)) \
		{                                                                            \
		};                                                                           \
	} while (0)

// 等待执行完成。一般用于转向循迹等不会超时的任务
#define ExcuteAndWait(action, Flag, waitStatus) \
	do                                          \
	{                                           \
		action;                                 \
		WaitForFlag(Flag, waitStatus);          \
		Stop();                                 \
	} while (0)


// 基本运动控制
void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_ByEncoder(int16_t digree);
void Track_ByEncoder(int speed, uint16_t setMP);
void Start_Tracking(int speed);

// 循迹线转弯（未做优化，不稳定）
void Turn_ByTrack(Driection_t dir);


// 自动执行
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next);
void Auto_RouteTask(RouteNode_t *current, RouteNode_t next);
void Auto_Run(void);

#endif // _MOVEMENT_H_
