#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "a_star.h"
#include "pid.h"
#include "route.h"
#include "malloc.h"
#include "debug.h"
#include "independent_task.h"

#define _USE_TRACK_TURNNING_IN_AUTO_RUN_ 1

// 全自动
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current)
{
	Generate_Routetask(routeTask, taskNumber); // 初始化坐标信息
	*current = routeTask[0].node;			   // 设定起始方向和坐标

	for (uint8_t i = 0; i < taskNumber; i++)
	{
		Auto_RouteTask(current, routeTask[i].node);
		if (routeTask[i].Task != NULL)
		{
			routeTask[i].Task(); // 任务非空时执行任务
		}
	}
}

// 从当前任务点行驶到下一个任务点
void Auto_RouteTask(RouteNode_t *current, RouteNode_t next)
{
	RouteNode_t *route = malloc(sizeof(RouteNode_t) * 12); // 两点间最多12途径点
	uint8_t routeCount = 0;

	A_Star_GetRouteBetweenNodes(*current, next, route, &routeCount);

	// 跳过第一个点，因为当前就在第一个点。
	for (uint8_t i = 1; i < routeCount; i++)
	{
		NextStatus = route[i];
		Go_ToNextNode(current, route[i]);
	}
	free(route);
}

// 行驶到下一个节点
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next)
{
	Direction_t finalDir = DIR_NOTSET;
	int8_t x = next.x - current->x;
	int8_t y = next.y - current->y;

	NextStatus = next; // 下一个坐标信息更新

	if ((x > 1 || y > 1 || x < -1 || y < -1) || (x != 0 && y != 0))
	{
		print_info("Node Skipped!!\r\n");
		return;
	}

	if (x > 0)
	{
		finalDir = DIR_RIGHT;
	}
	else if (x < 0)
	{
		finalDir = DIR_LEFT;
	}
	else if (y > 0)
	{
		finalDir = DIR_UP;
	}
	else if (y < 0)
	{
		finalDir = DIR_DOWN;
	}
	else
	{
		print_info("Same Node\r\n"); // 同一点
		return;
	}

	// 当前点非十字线时转向使用码盘转向
	if ((current->x % 2 == 0) || (current->y % 2 == 0))
	{
		Turn_ToDirection(&current->dir, finalDir, TurnMethod_Encoder);
	}
	else
	{
		Turn_ToDirection(&current->dir, finalDir, TurnMethod_Track);
	}

	if (next.x % 2 == 0) // X轴为偶数的坐标
	{
		// 横坐标0和6的为两侧车库，码盘值需要变小
		uint16_t encoderValue = ((next.x == 0) || (next.x == 6)) ? SidePark_Value : LongTrack_Value;

		ExcuteAndWait(Track_ByEncoder(Track_Speed, encoderValue), Stop_Flag, FORBACKCOMPLETE);
	}
	else if (next.y % 2 == 0) // Y轴为偶数的坐标
	{
		ExcuteAndWait(Track_ByEncoder(Track_Speed, ShortTrack_Value), Stop_Flag, FORBACKCOMPLETE);
	}
	else // 前方十字路口
	{
		// 循迹到十字路口
		Start_Tracking(Track_Speed);
		WaitForFlag(Stop_Flag, CROSSROAD);

		// 行驶到十字路口中心
		// ExcuteAndWait(Go_Ahead(Track_Speed, ToCrossroadCenter), Stop_Flag, FORBACKCOMPLETE);

		// 应对放在十字线后面的白卡
		Go_Ahead(Track_Speed, ToCrossroadCenter);
		Submit_SpeedChanges(); // 提交速度更改

		while (Stop_Flag != FORBACKCOMPLETE)
		{
			extern uint8_t RFID_RoadSection;
			extern uint8_t FOUND_RFID_CARD;

			if (RFID_RoadSection && (FOUND_RFID_CARD == false))
			{
				Get_Track();
				if (IS_All_WHITE())
				{
					// DEBUG_PIN_2_SET();

					FOUND_RFID_CARD = true;		  // 找到白卡
					Save_StatusBeforeFoundRFID(); // 保存当前状态
					Stop();						  // 暂停运行
					Submit_SpeedChanges();		  // 提交速度更改
					TIM_Cmd(TIM5, ENABLE);		  // 使能RFID处理定时器
				}
			}
		};
		Stop();
	}

	// 更新当前位置信息和方向
	current->x = next.x;
	current->y = next.y;
	current->dir = finalDir;
}

// 基本运动控制函数↓↓

uint32_t lastStopStamp = 0;
// 停止运行，清空标志位，清空PID数据
void Stop(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	// Mp_Value = 0;
	Update_MotorSpeed(0, 0);
	PidData_Clear();
	lastStopStamp = Get_GlobalTimeStamp(); // warning
}

// 停止但不清空PID数据
void Stop_WithoutPIDClear(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	Update_MotorSpeed(0, 0);
	lastStopStamp = Get_GlobalTimeStamp();
}

// 前后移动 单位厘米 正负方向
void Move_ByEncoder(int speed, float distance)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;

	if (distance > 0)
	{
		temp_MP = distance * Centimeter_Value;
		Moving_ByEncoder = ENCODER_GO;
		Update_MotorSpeed(speed, speed);
	}
	else
	{
		temp_MP = -distance * Centimeter_Value;
		Moving_ByEncoder = ENCODER_BACK;
		Update_MotorSpeed(-speed, -speed);
	}
}

// 前进
void Go_Ahead(int speed, uint16_t encoderValue)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_GO;
	temp_MP = encoderValue;
	Track_Mode = TrackMode_NONE;
	Update_MotorSpeed(speed, speed);
}

// 后退
void Back_Off(int speed, uint16_t encoderValue)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_BACK;
	temp_MP = encoderValue;
	Track_Mode = TrackMode_NONE;
	Update_MotorSpeed(-speed, -speed);
}

// 开始循迹
void Start_Tracking(int speed)
{
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NORMAL;
	Moving_ByEncoder = ENCODER_NONE;
	Car_Speed = speed;
}

// 根据码盘设定值循迹
void Track_ByEncoder(int speed, uint16_t setMP)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_ENCODER;
	temp_MP = setMP;
	Car_Speed = speed;
}

// 根据码盘值转任意角度
void Turn_ByEncoder(int16_t digree)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_TurnByValue;
	if (digree >= 0)
	{
		Update_MotorSpeed(Turn_Speed, -Turn_Speed);
		TurnByEncoder_Value = digree * ClockWiseDigreeToEncoder;
	}
	else
	{
		Update_MotorSpeed(-Turn_Speed, Turn_Speed);
		TurnByEncoder_Value = -digree * CountClockWiseDigreeToEncoder;
	}
}

// 转到下一个循迹线
uint8_t turnLeftOrRightt = DIR_NOTSET;
void Turn_ByTrack(Direction_t dir)
{
	if ((dir != DIR_RIGHT) && (dir != DIR_LEFT))
	{
		print_info("Turn Dir ERROR\r\n");
		return;
	}
	turnLeftOrRightt = dir;

	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_Turn;
	Moving_ByEncoder = ENCODER_NONE;

	if (dir == DIR_RIGHT)
	{
		Update_MotorSpeed(Turn_Speed, -Turn_Speed);
	}
	else if (dir == DIR_LEFT)
	{
		Update_MotorSpeed(-Turn_Speed, Turn_Speed);
	}
}

#define TURN_ONCE(dir) Turn_Once(dir)
#define TURN_TWICE(dir) \
	Turn_Once(dir);     \
	Turn_Once(dir)

void TurnOnce_TrackMethod(Direction_t dir)
{
	ExcuteAndWait(Turn_ToNextTrack(dir), Stop_Flag, TURNCOMPLETE);
}

void TurnOnce_EncoderMethod(Direction_t dir)
{
	TURN((dir == DIR_LEFT) ? -90 : 90);
}

// 转到特定方向并更新当前方向
void Turn_ToDirection(int8_t *current, Direction_t target, TurnMethod_t turnMethod)
{
	void (*Turn_Once)(Direction_t) = (turnMethod == TurnMethod_Track) ? TurnOnce_TrackMethod : TurnOnce_EncoderMethod;

	switch (*current)
	{
	case DIR_UP:
		switch (target)
		{
		case DIR_UP:
			break;
		case DIR_DOWN:
			TURN_TWICE(DIR_RIGHT);
			break;
		case DIR_LEFT:
			TURN_ONCE(DIR_LEFT);
			break;
		case DIR_RIGHT:
			TURN_ONCE(DIR_RIGHT);
			break;
		default:
			break;
		}
		break;

	case DIR_DOWN:
		switch (target)
		{
		case DIR_UP:
			TURN_TWICE(DIR_RIGHT);
			break;
		case DIR_DOWN:
			break;
		case DIR_LEFT:
			TURN_ONCE(DIR_RIGHT);
			break;
		case DIR_RIGHT:
			TURN_ONCE(DIR_LEFT);
			break;
		default:
			break;
		}
		break;

	case DIR_LEFT:
		switch (target)
		{
		case DIR_UP:
			TURN_ONCE(DIR_RIGHT);
			break;
		case DIR_DOWN:
			TURN_ONCE(DIR_LEFT);
			break;
		case DIR_LEFT:
			break;
		case DIR_RIGHT:
			TURN_TWICE(DIR_RIGHT);
			break;
		default:
			break;
		}
		break;

	case DIR_RIGHT:
		switch (target)
		{
		case DIR_UP:
			TURN_ONCE(DIR_LEFT);
			break;
		case DIR_DOWN:
			TURN_ONCE(DIR_RIGHT);
			break;
		case DIR_LEFT:
			TURN_TWICE(DIR_RIGHT);
			break;
		case DIR_RIGHT:
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
	*current = target;
}
