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

// 等待某个标志位。注意：此操作没有超时处理机制
#define WaitForFlag(flag, status) \
	do                            \
	{                             \
		while (flag != status)    \
		{                         \
		};                        \
	} while (0)

// 等待动作完成，用于转向循迹等不会超时的任务
#define ExcuteAndWait(action, Flag, waitStatus) \
	do                                          \
	{                                           \
		action;                                 \
		WaitForFlag(Flag, waitStatus);          \
		Stop();                                 \
	} while (0)

// 快速动作宏定义
#define TURN(digree) ExcuteAndWait(Turn_ByEncoder(digree), Stop_Flag, TURNCOMPLETE)
#define MOVE(distance) ExcuteAndWait(Move_ByEncoder(Mission_Speed, distance), Stop_Flag, FORBACKCOMPLETE)
#define TURN_TO(target) Turn_ToDirection(&CurrentStaus.dir, target)

// 基本运动控制
void Move_ByEncoder(int speed, float distance);

void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_ByEncoder(int16_t digree);
void Track_ByEncoder(int speed, uint16_t setMP);
void Start_Tracking(int speed);

// 循迹线转弯
void Turn_ByTrack(Direction_t dir);
#define Turn_ToNextTrack(dir) Turn_ByTrack(dir)
void Turn_ToDirection(int8_t *current, Direction_t target);

// 自动执行
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next);
void Auto_RouteTask(RouteNode_t *current, RouteNode_t next);
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current);

#endif // _MOVEMENT_H_
