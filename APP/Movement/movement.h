#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"
#include "Timer.h"
#include "route.h"

// Stop_Flag的枚举
typedef enum
{
	TRACKING = 0x00,		// 循迹状态
	CROSSROAD = 0x01,		// 十字路口
	TURNCOMPLETE = 0x02,	// 转弯完成
	FORBACKCOMPLETE = 0x03, // 前进后退完成(定长循迹)
	OUTTRACK = 0x04			// 出线
} StopFlag_t;

// 定义转向模式
typedef enum TurnMethod_Struct
{
	TurnMethod_Track = 0,
	TurnMethod_Encoder = 1
} TurnMethod_t;

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

// 通过码盘转向，不自动记录
#define TURN(digree) ExcuteAndWait(Turn_ByEncoder(digree), Stop_Flag, TURNCOMPLETE)
// 前后移动
#define MOVE(distance) ExcuteAndWait(Move_ByEncoder(Mission_Speed, distance), Stop_Flag, FORBACKCOMPLETE)
// 根据循迹线转到某个方向，自动记录方向变化
#define TURN_TO(target) Turn_ToDirection(&CurrentStatus.dir, target, TurnOnce_TrackMethod)

// 跳跃节点控制
extern int8_t skipNodes;

// 基本运动控制
void Move_ByEncoder(int speed, float distance);

void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_ByEncoder(int16_t digree);
void Track_ByEncoder(int speed, uint16_t setMP);
void Start_Tracking(int speed);
void Stop_WithoutPIDClear(void);

void TurnOnce_TrackMethod(Direction_t dir);
void TurnOnce_EncoderMethod(Direction_t dir);

// 循迹线转弯
void Turn_ByTrack(Direction_t dir);

// 自动执行
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next);
void Auto_DriveBetweenNodes(RouteNode_t *current, RouteNode_t next);
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current);

void Reverse_Parcking(RouteNode_t *current, uint8_t targetGarage[3]);
void Auto_ReverseParcking(RouteNode_t *current, uint8_t targetGarage[3], void(*taskAfterParcking)(void));

#endif // _MOVEMENT_H_
