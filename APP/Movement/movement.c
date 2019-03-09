#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"
#include "a_star.h"
#include "pid.h"
#include "task.h"
#include "route.h"


// ������ȫ�Զ�
void Auto_Run(void)
{
	CurrentStaus = Route_Task[0].node; // ��ʼ����ǰλ��

	uint8_t count = ROUTE_TASK_NUMBER;
	for (size_t i = 0; i < count; i++)
	{
		if (RouteTask_Finished[i] == 0)
		{
			Auto_RouteTask(CurrentStaus, i);
		}
	}

	// ���ֲ�������·���������������
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
	else if (taskN == 0) // ����Ϊ�㣬ִ�г�ʼ����
	{
		Go_ToNextNode(Route_Task[taskN]);
	}

	RouteTask_Finished[taskN] = 1;
}

// ��ʻ����һ���ڵ�
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
		if (next.Task != NULL) // ����Ƿ�������
		{
			next.Task();
		}
		return;
	}

	if ((finalDir == DIR_RIGHT || finalDir == DIR_LEFT))
	{
		switch (CurrentStaus.dir)
		{
		case DIR_UP:
			(finalDir == DIR_RIGHT) ? Turn_ByEncoder(90) : Turn_ByEncoder(-90);
			break;
		case DIR_DOWN:
			(finalDir == DIR_RIGHT) ? Turn_ByEncoder(-90) : Turn_ByEncoder(90);
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
			(finalDir == DIR_UP) ? Turn_ByEncoder(90) : Turn_ByEncoder(-90);
			break;
		case DIR_RIGHT:
			(finalDir == DIR_UP) ? Turn_ByEncoder(-90) : Turn_ByEncoder(90);
			break;
		default:
			print_info("CurrentDir NOT SET!\r\n");
			break;
		}
	}

	if (Moving_ByEncoder != ENCODER_NONE) // ��ת�����񣬵ȴ�
	{
		WaitForFlag(Stop_Flag, TURNCOMPLETE);
	}

	if (next.node.x % 2 == 0) // X��Ϊż��������
	{
		Track_ByEncoder(Track_Speed, LongTrack_Value);
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}
	else if (next.node.y % 2 == 0) // Y��Ϊż��������
	{
		Track_ByEncoder(Track_Speed, ShortTrack_Value);
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}
	else // ǰ��ʮ��·��
	{
		Start_Tracking(Track_Speed); // ѭ����ʮ��·��
		WaitForFlag(Stop_Flag, CROSSROAD);
		Go_Ahead(Track_Speed, ToCrossroadCenter); // ǰ����ʮ��·������
		WaitForFlag(Stop_Flag, FORBACKCOMPLETE);
		Stop();
	}

	// ���µ�ǰλ����Ϣ��״̬
	CurrentStaus.x = next.node.x;
	CurrentStaus.y = next.node.y;
	CurrentStaus.dir = finalDir;

	if (next.Task != NULL) // ����Ƿ�������
	{
		next.Task();
	}
}

// ֹͣ���У���ձ�־λ�����PID����
void Stop(void)
{
	Roadway_Flag_clean(); //�����־λ״̬
	// Mp_Value = 0;
	Control(0, 0);
	PidData_Clear();
	Beep(1);
}

// ǰ��
void Go_Ahead(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_GO;
	temp_MP = mp;
	Track_Mode = TrackMode_NONE;
	Control(speed, speed);
}

// ����
void Back_Off(int speed, uint16_t mp)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_BACK;
	temp_MP = mp;
	Track_Mode = TrackMode_NONE;
	Control(-speed, -speed);
}

// ��ʼѭ��
void Start_Tracking(int speed)
{
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NORMAL;
	Moving_ByEncoder = ENCODER_NONE;
	Car_Speed = speed;
	Control(Car_Speed, Car_Speed);
}

// ���������趨ֵѭ��
void Track_ByEncoder(int speed, uint16_t setMP)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_ENCODER;
	temp_MP = setMP;
	Car_Speed = speed;
}

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