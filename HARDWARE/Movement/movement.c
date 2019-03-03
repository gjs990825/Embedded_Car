#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"
#include "a_star.h"
#include "pid.h"

#define WaitForFlag(flag, status) while (flag != status){}

// 测试用
void autorun(void)
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

	CurrentStaus.x = 1;
	CurrentStaus.y = 0;
	CurrentStaus.dir = DIR_UP;

	for(uint8_t i = 0; i < Final_StepCount; i++)
	{
		print_info("NOW:(%d,%d)\r\n", Final_Route[i].x, Final_Route[i].y);
		Go_ToNextNode(Final_Route[i]);
	}
}

// 行驶到下一个节点
void Go_ToNextNode(RouteNode next)
{
	int8_t finalDir = 0;
	int8_t x = next.x - CurrentStaus.x;
	int8_t y = next.y - CurrentStaus.y;

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
		print_info("FIN DIR ERROR\r\n");
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

	if (Moving_ByEncoder != ENCODER_NONE)
	{
		WaitForFlag(Stop_Flag, TURNCOMPLETE);
	}

	Start_Tracking(Track_Speed);
	WaitForFlag(Stop_Flag, CROSSROAD);
	Stop();

	Go_Ahead(Track_Speed, ToCrossroadCenter);
	WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
	Stop();

	CurrentStaus.x = next.x;
	CurrentStaus.y = next.y;
	CurrentStaus.dir = finalDir;
}

// 停止运行，清空标志位，清空PID数据
void Stop(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	// Mp_Value = 0;
	Control(0, 0);
	PidData_Clear();
	// Beep(1);
}

// 前进
void Go_Ahead(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_GO;
	temp_MP = mp;
	Car_Speed = speed;
	Track_Mode = TrackMode_NONE;
	Control(Car_Speed, Car_Speed);
}

// 后退
void Back_Off(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_BACK;
	temp_MP = mp;
	Car_Speed = speed;
	Track_Mode = TrackMode_NONE;
	Control(-Car_Speed, -Car_Speed);
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

void Turn_Left45(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_LEFT45;
	Car_Speed = Turn_Speed;
	Control(-Car_Speed, Car_Speed);
}

void Turn_Left90(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_LEFT90;
	Car_Speed = Turn_Speed;
	Control(-Car_Speed, Car_Speed);
}

void Turn_Right45(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT45;
	Car_Speed = Turn_Speed;
	Control(Car_Speed, -Car_Speed);
}

void Turn_Right90(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT90;
	Car_Speed = Turn_Speed;
	Control(Car_Speed, -Car_Speed);
}

void Turn_Right180(void)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_RIGHT180;
	Car_Speed = Turn_Speed;
	Control(Car_Speed, -Car_Speed);
}


// void Turn_Left90(void)
// {
// 	Roadway_mp_syn();
// 	//	Mp_Value = Roadway_mp_Get();
// 	Stop_Flag = TRACKING;
// 	Go_Flag = 0;
	
	
// 	wheel_Nav_Flag = 0;
// 	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 1;
// 	Right180_Flag = 0; //转弯控制
// 	Back_Flag = 0;
// 	Track_Flag = 0;
// 	Car_Speed = Turn_Speed;
// 	Control(-Car_Speed, Car_Speed);
// 	//  TIM_Cmd(TIM9,ENABLE);
// }

// void Turn_Right45(void)
// {
// 	Roadway_mp_syn();
// 	//	Mp_Value = Roadway_mp_Get();
// 	Stop_Flag = TRACKING;
// 	Go_Flag = 0;
	
	
// 	wheel_Nav_Flag = 0;
// 	Right45_Flag = 1, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
// 	Right180_Flag = 0; //转弯控制
// 	Back_Flag = 0;
// 	Track_Flag = 0;
// 	Car_Speed = Turn_Speed;
// 	Control(Car_Speed, -Car_Speed);
// 	//	TIM_Cmd(TIM9,ENABLE);
// }

// void Turn_Right90(void)
// {
// 	Roadway_mp_syn();
// 	//	Mp_Value = Roadway_mp_Get();
// 	Stop_Flag = TRACKING;
// 	Go_Flag = 0;
	
	
// 	wheel_Nav_Flag = 0;
// 	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 1, Left90_Flag = 0;
// 	Right180_Flag = 0; //转弯控制
// 	Back_Flag = 0;
// 	Track_Flag = 0;
// 	Car_Speed = Turn_Speed;
// 	Control(Car_Speed, -Car_Speed);
// 	//	TIM_Cmd(TIM9,ENABLE);
// }

// void Turn_Right180(void)
// {
// 	Roadway_mp_syn();
// 	//	Mp_Value = Roadway_mp_Get();
// 	Stop_Flag = TRACKING;
// 	Go_Flag = 0;
	
	
// 	wheel_Nav_Flag = 0;
// 	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
// 	Right180_Flag = 1; //转弯控制
// 	Back_Flag = 0;
// 	Track_Flag = 0;
// 	Car_Speed = Turn_Speed;
// 	Control(Car_Speed, -Car_Speed);
// 	//  TIM_Cmd(TIM9,ENABLE);
// }

