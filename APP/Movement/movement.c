#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"
#include "a_star.h"
#include "pid.h"
#include "task.h"
#include "route.h"

// 测试用全自动
void Auto_Run(void)
{
	CurrentStaus = Route_Task[0].node; // 初始化当前位置

	uint8_t count = ROUTE_TASK_NUMBER;
	for (size_t i = 0; i < count; i++)
	{
		if (RouteTask_Finished[i] == 0)
		{
			Auto_RouteTask(CurrentStaus, i);
		}
	}

	// 此种不允许从路径点外的坐标点出发
	// A_Star_GetRoute();
	// for(uint8_t i = 0; i < Final_StepCount; i++)
	// {
	// 	print_info("NOW:(%d,%d)\r\n", Final_Route[i].node.x, Final_Route[i].node.y);
	// 	Go_ToNextNode(Final_Route[i]);
	// }
}

void Auto_RouteTask(RouteNode current, uint8_t taskN)
{
	print_info("Route %s\r\n", (A_Star_GetRouteBewteenTasks(current, Route_Task[taskN]) == true) ? "OK" : "ERROR OR SAME");
	if (Final_StepCount != 0)
	{
		for (uint8_t i = 0; i < Final_StepCount; i++)
		{
			print_info("NOW:(%d,%d)\r\n", Final_Route[i].node.x, Final_Route[i].node.y);
			Go_ToNextNode(Final_Route[i]);
		}
	}
	else if (taskN == 0) // 步数为零，执行初始任务
	{
		Go_ToNextNode(Route_Task[taskN]);
	}

	RouteTask_Finished[taskN] = 1;
}

// 行驶到下一个节点
void Go_ToNextNode(Route_Task_t next)
{
	int8_t finalDir = 0;
	int8_t x = next.node.x - CurrentStaus.x;
	int8_t y = next.node.y - CurrentStaus.y;

	NextStatus = next.node; // 下一个坐标信息更新

	if ((x > 1 || y > 1) || (x != 0 && y != 0))
	{
		print_info("NODE ERROR!!\r\n");
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
		print_info("Same\r\n"); // 同一点
		if (next.Task != NULL)  // 检查是否有任务
		{
			next.Task();
		}
		return;
	}

	// if ((finalDir == DIR_RIGHT || finalDir == DIR_LEFT))
	// {
	// 	switch (CurrentStaus.dir)
	// 	{
	// 	case DIR_UP:
	// 		Turn_ByTrack((finalDir == DIR_RIGHT) ? (DIR_RIGHT) : (DIR_LEFT));
	// 		break;
	// 	case DIR_DOWN:
	// 		Turn_ByTrack((finalDir == DIR_RIGHT) ? (DIR_LEFT) : (DIR_RIGHT));
	// 		break;
	// 	case DIR_LEFT:
	// 		(finalDir == DIR_RIGHT) ? Turn_ByEncoder(180) : (void)0;
	// 		break;
	// 	case DIR_RIGHT:
	// 		(finalDir == DIR_RIGHT) ? (void)0 : Turn_ByEncoder(180);
	// 		break;
	// 	default:
	// 		print_info("CurrentDir NOT SET!\r\n");
	// 		break;
	// 	}
	// }
	// else
	// {
	// 	switch (CurrentStaus.dir)
	// 	{
	// 	case DIR_UP:
	// 		(finalDir == DIR_UP) ? (void)0 : Turn_ByEncoder(180);
	// 		break;
	// 	case DIR_DOWN:
	// 		(finalDir == DIR_UP) ? Turn_ByEncoder(180) : (void)0;
	// 		break;
	// 	case DIR_LEFT:
	// 		Turn_ByTrack((finalDir == DIR_UP) ? (DIR_RIGHT) : (DIR_LEFT));
	// 		break;
	// 	case DIR_RIGHT:
	// 		Turn_ByTrack((finalDir == DIR_UP) ? (DIR_LEFT) : (DIR_RIGHT));
	// 		break;
	// 	default:
	// 		print_info("CurrentDir NOT SET!\r\n");
	// 		break;
	// 	}
	// }

	if ((finalDir == DIR_RIGHT || finalDir == DIR_LEFT))
	{
		switch (CurrentStaus.dir)
		{
		case DIR_UP:
			Turn_ByEncoder((finalDir == DIR_RIGHT) ? (90) : (-90));
			break;
		case DIR_DOWN:
			Turn_ByEncoder((finalDir == DIR_RIGHT) ? (-90) : (90));
			break;
		case DIR_LEFT:
			(finalDir == DIR_RIGHT) ? Turn_ByEncoder(180) : (void)0;
			break;
		case DIR_RIGHT:
			(finalDir == DIR_RIGHT) ? (void)0 : Turn_ByEncoder(180);
			break;
		default:
			print_info("CurrentDir NOT SET!\r\n");
			break;
		}
	}
	else
	{
		switch (CurrentStaus.dir)
		{
		case DIR_UP:
			(finalDir == DIR_UP) ? (void)0 : Turn_ByEncoder(180);
			break;
		case DIR_DOWN:
			(finalDir == DIR_UP) ? Turn_ByEncoder(180) : (void)0;
			break;
		case DIR_LEFT:
			Turn_ByEncoder((finalDir == DIR_UP) ? (90) : (-90));
			break;
		case DIR_RIGHT:
			Turn_ByEncoder((finalDir == DIR_UP) ? (-90) : (90));
			break;
		default:
			print_info("CurrentDir NOT SET!\r\n");
			break;
		}
	}

	if ((Moving_ByEncoder != ENCODER_NONE) || (Track_Mode == TrackMode_Turn)) // 有转向任务，等待完成
	{
		WaitForFlag(Stop_Flag, TURNCOMPLETE);
	}

	if (next.node.x % 2 == 0) // X轴为偶数的坐标
	{
		ExcuteAndWait(Track_ByEncoder(Track_Speed, LongTrack_Value), Stop_Flag, FORBACKCOMPLETE);
	}
	else if (next.node.y % 2 == 0) // Y轴为偶数的坐标
	{
		ExcuteAndWait(Track_ByEncoder(Track_Speed, ShortTrack_Value), Stop_Flag, FORBACKCOMPLETE);
	}
	else // 前方十字路口
	{
		Start_Tracking(Track_Speed); // 循迹到十字路口
		WaitForFlag(Stop_Flag, CROSSROAD);
		ExcuteAndWait(Go_Ahead(Track_Speed, ToCrossroadCenter), Stop_Flag, FORBACKCOMPLETE); // 前进到十字路口中心
	}

	// 更新当前位置信息和方向
	CurrentStaus.x = next.node.x;
	CurrentStaus.y = next.node.y;
	CurrentStaus.dir = finalDir;

	if (next.Task != NULL) // 检查是否有任务
	{
		next.Task();
	}
}

// 停止运行，清空标志位，清空PID数据
void Stop(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	// Mp_Value = 0;
	Control(0, 0);
	PidData_Clear();
	Beep(1);
}

// 前进
void Go_Ahead(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_GO;
	temp_MP = mp;
	Track_Mode = TrackMode_NONE;
	Control(speed, speed);
}

// 后退
void Back_Off(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_BACK;
	temp_MP = mp;
	Track_Mode = TrackMode_NONE;
	Control(-speed, -speed);
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
		Control(Turn_Speed, -Turn_Speed);
		TurnByEncoder_Value = digree * ClockWiseDigreeToEncoder;
	}
	else
	{
		Control(-Turn_Speed, Turn_Speed);
		TurnByEncoder_Value = -digree * CountClockWiseDigreeToEncoder;
	}
}

extern uint8_t TrackStatus;

void Turn_ByTrack(Driection_t dir)
{
	if ((dir != DIR_RIGHT) && (dir != DIR_LEFT))
	{
		return;
	}

	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_Turn;
	Moving_ByEncoder = ENCODER_NONE;

	TrackStatus = 0; // 清空标志位
	if (dir == DIR_RIGHT)
	{
		Control(Turn_Speed, -Turn_Speed);
	}
	else if (dir == DIR_LEFT)
	{
		Control(-Turn_Speed, Turn_Speed);
	}
}
