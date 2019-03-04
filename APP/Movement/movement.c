#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"
#include "a_star.h"
#include "pid.h"
#include "task.h"

#define WaitForFlag(flag, status) \
	while (flag != status)        \
	{                             \
	}

// 测试用
void Auto_Run(void)
{
	// Start_Tracking(Track_Speed);
	// WaitForFlag(Stop_Flag, CROSSROAD);
	// Stop();

	// // Go_Ahead(50, ToCrossroadCenter);
	// // WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	// // Stop();

	// Go_Ahead(Track_Speed, ToCrossroadCenter);
	// WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	// Stop();

	// Track_ByEncoder(Track_Speed, LongTrack_Value);
	// WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	// Stop();

	// Start_Tracking(Track_Speed);
	// WaitForFlag(Stop_Flag, CROSSROAD);
	// Go_Ahead(Track_Speed, ToCrossroadCenter);
	// WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	// Stop();

	// Track_ByEncoder(Track_Speed, LongTrack_Value);
	// WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	// Stop();

	CurrentStaus = Route_Task[0].node; // 初始化当前位置


	// 走自动规划的路径
	for(uint8_t i = 0; i < Final_StepCount; i++)
	{
		print_info("NOW:(%d,%d)\r\n", Final_Route[i].node.x, Final_Route[i].node.y);
		Go_ToNextNode(Final_Route[i]);
	}
}

// int8_t Check_Task(int8_t coordinate[2])
// {
// 	int8_t count = (sizeof(Route_Task) / sizeof(Route_Task[0]));
// 	for(size_t i = 0; i < count; i++)
// 	{
// 		if ((Route_Task[i].node.x == coordinate[0]) && (Route_Task[i].node.y == coordinate[1]))
// 		{
// 			return i;
// 		}
// 	}
// 	return -1;
// }

// 行驶到下一个节点
void Go_ToNextNode(Route_Task_t next)
{
	int8_t finalDir = 0;
	int8_t x = next.node.x - CurrentStaus.x;
	int8_t y = next.node.y - CurrentStaus.y;

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
		print_info("Same coordinate\r\n");
		return;
	}

	if ((finalDir == DIR_RIGHT || finalDir == DIR_LEFT))
	{
		switch (CurrentStaus.dir)
		{
		case DIR_UP:
			(finalDir == DIR_RIGHT) ? Turn_Right90() : Turn_Left90();
			break;
		case DIR_DOWN:
			(finalDir == DIR_RIGHT) ? Turn_Left90() : Turn_Right90();
			break;
		case DIR_LEFT:
			(finalDir == DIR_RIGHT) ? Turn_Right180() : (void)0;
			break;
		case DIR_RIGHT:
			(finalDir == DIR_RIGHT) ? (void)0 : Turn_Right180();
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
			(finalDir == DIR_UP) ? (void)0 : Turn_Right180();
			break;
		case DIR_DOWN:
			(finalDir == DIR_UP) ? Turn_Right180() : (void)0;
			break;
		case DIR_LEFT:
			(finalDir == DIR_UP) ? Turn_Right90() : Turn_Left90();
			break;
		case DIR_RIGHT:
			(finalDir == DIR_UP) ? Turn_Left90() : Turn_Right90();
			break;
		default:
			print_info("CurrentDir NOT SET!\r\n");
			break;
		}
	}

	if (Moving_ByEncoder != ENCODER_NONE) // 有转向任务，等待
	{
		WaitForFlag(Stop_Flag, TURNCOMPLETE);
	}

	if (next.node.x % 2 == 0) // X轴为偶数的坐标
	{
		Track_ByEncoder(Track_Speed, LongTrack_Value);
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}
	else if (next.node.y % 2 == 0) // Y轴为偶数的坐标
	{
		Track_ByEncoder(Track_Speed, ShortTrack_Value);
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}
	else // 前方十字路口
	{
		Start_Tracking(Track_Speed); // 循迹到十字路口
		WaitForFlag(Stop_Flag, CROSSROAD);
		Go_Ahead(Track_Speed, ToCrossroadCenter); // 前进到十字路口中心
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}

	// 更新当前位置信息和状态
	CurrentStaus.x = next.node.x;
	CurrentStaus.y = next.node.y;
	CurrentStaus.dir = finalDir;

	// CurrentStaus = next.node;
	// CurrentStaus.dir = finalDir;

	if (next.Task != NULL) // 检查是否有任务
	{
		next.Task();
	}
	
	
	// int8_t n = Check_Task(next); // 检查是否有任务
	// if (n != -1)
	// {
	// 	Route_Task[n].Task();
	// }
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
	Control(Car_Speed, Car_Speed);
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

void Turn_Left45(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_LEFT45;
	Control(-Turn_Speed, Turn_Speed);
}

void Turn_Left90(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_LEFT90;
	Control(-Turn_Speed, Turn_Speed);
}

void Turn_Right45(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT45;
	Control(Turn_Speed, -Turn_Speed);
}

void Turn_Right90(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT90;
	Control(Turn_Speed, -Turn_Speed);
}

void Turn_Right180(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT180;
	Control(Turn_Speed, -Turn_Speed);
}
