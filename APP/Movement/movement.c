#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"
#include "a_star.h"
#include "pid.h"
#include "task.h"
#include "route.h"
#include "stdlib.h"

// ȫ�Զ�
void Auto_Run(void)
{
	Generate_Routetask(Route_Task, ROUTE_TASK_NUMBER); // ��ʼ��������Ϣ
	CurrentStaus = Route_Task[0].node;					   // �趨��ʼ���������

	for (uint8_t i = 0; i < ROUTE_TASK_NUMBER; i++)
	{
		Auto_RouteTask(&CurrentStaus, Route_Task[i].node);
		if (Route_Task[i].Task != NULL)
		{
			Route_Task[i].Task(); // ����ǿ�ʱִ������
		}
	}
}

// �ӵ�ǰ�������ʻ����һ�������
void Auto_RouteTask(RouteNode_t *current, RouteNode_t next)
{
	RouteNode_t *route = mymalloc(SRAMIN, sizeof(RouteNode_t) * 12); // ��������12;����
	uint8_t routeCount = 0;

	A_Star_GetTestRoute(*current, next, route, &routeCount);
	print_info("routeCount = %d\r\n", routeCount);

	// ������һ���㣬��Ϊ��ǰ���ڵ�һ���㡣
	for (uint8_t i = 1; i < routeCount; i++)
	{
		NextStatus = route[i];
		Go_ToNextNode(current, route[i]);
	}
	myfree(SRAMIN, route);
}

// ��ʻ����һ���ڵ�
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next)
{
	int8_t finalDir = 0;
	int8_t x = next.x - current->x;
	int8_t y = next.y - current->y;

	NextStatus = next; // ��һ��������Ϣ����

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
		print_info("Same Node\r\n"); // ͬһ��
		return;
	}

	if ((finalDir == DIR_RIGHT || finalDir == DIR_LEFT))
	{
		switch (current->dir)
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
		switch (current->dir)
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

	if ((Moving_ByEncoder != ENCODER_NONE) || (Track_Mode == TrackMode_Turn)) // ��ת�����񣬵ȴ����
	{
		WaitForFlag(Stop_Flag, TURNCOMPLETE);
	}

	if (next.x % 2 == 0) // X��Ϊż��������
	{
		ExcuteAndWait(Track_ByEncoder(Track_Speed, LongTrack_Value), Stop_Flag, FORBACKCOMPLETE);
	}
	else if (next.y % 2 == 0) // Y��Ϊż��������
	{
		ExcuteAndWait(Track_ByEncoder(Track_Speed, ShortTrack_Value), Stop_Flag, FORBACKCOMPLETE);
	}
	else // ǰ��ʮ��·��
	{
		Start_Tracking(Track_Speed); // ѭ����ʮ��·��
		WaitForFlag(Stop_Flag, CROSSROAD);
		ExcuteAndWait(Go_Ahead(Track_Speed, ToCrossroadCenter), Stop_Flag, FORBACKCOMPLETE); // ǰ����ʮ��·������
	}

	// ���µ�ǰλ����Ϣ�ͷ���
	current->x = next.x;
	current->y = next.y;
	current->dir = finalDir;
}

// �����˶����ƺ�������

// ֹͣ���У���ձ�־λ�����PID����
void Stop(void)
{
	Roadway_Flag_clean(); //�����־λ״̬
	// Mp_Value = 0;
	Control(0, 0);
	PidData_Clear();
	delay_ms(70); // warning
}

// ǰ���ƶ� ��λ���� ��������
void Move_ByEncoder(int speed, int16_t distance)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	temp_MP = abs(distance * Centimeter_Value);
	Track_Mode = TrackMode_NONE;

	if (distance > 0)
	{
		Control(speed, speed);
		Moving_ByEncoder = ENCODER_GO;
	}
	else
	{
		Control(-speed, -speed);
		Moving_ByEncoder = ENCODER_BACK;
	}
}

// ǰ��
void Go_Ahead(int speed, uint16_t encoderValue)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_GO;
	temp_MP = encoderValue;
	Track_Mode = TrackMode_NONE;
	Control(speed, speed);
}

// ����
void Back_Off(int speed, uint16_t encoderValue)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Moving_ByEncoder = ENCODER_BACK;
	temp_MP = encoderValue;
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

// ��������ֵת����Ƕ�
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
// ת����һ��ѭ���ߣ���Ҫ�Ż���
void Turn_ByTrack(Driection_t dir)
{
	if ((dir != DIR_RIGHT) && (dir != DIR_LEFT))
	{
		return;
	}

	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_Turn;
	Moving_ByEncoder = ENCODER_NONE;

	TrackStatus = 0; // ��ձ�־λ
	if (dir == DIR_RIGHT)
	{
		Control(Turn_Speed, -Turn_Speed);
	}
	else if (dir == DIR_LEFT)
	{
		Control(-Turn_Speed, Turn_Speed);
	}
}
