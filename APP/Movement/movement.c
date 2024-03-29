#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "a_star.h"
#include "pid.h"
#include "route.h"
#include "malloc.h"
#include "debug.h"
#include "independent_task.h"

// 全自动
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current)
{
	Generate_Routetask(routeTask, taskNumber); // 初始化坐标信息
	*current = routeTask[0].node;			   // 设定起始方向和坐标

	for (uint8_t i = 0; i < taskNumber; i++)
	{
		Auto_DriveBetweenNodes(current, routeTask[i].node);
		if (routeTask[i].Task != NULL)
		{
			routeTask[i].Task(); // 任务非空时执行任务
		}
	}
}

// 从当前点自动寻路到下一个点
// 两点可间隔，但要在合法坐标点上
void Auto_DriveBetweenNodes(RouteNode_t *current, RouteNode_t next)
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

int8_t skipNodes = 0;
// 行驶到下一个节点
// 两个节点必须相邻
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next)
{
	// 节点跳过操作，请注意前后状态对应
	if (skipNodes > 0)
	{
		skipNodes--;

		// 跳节点一般不改变方向
		// 更新坐标状态信息
		NextStatus = next;
		current->x = next.x;
		current->y = next.y;
		return;
	}

	// 检查计算参数

	Direction_t finalDir = DIR_NOTSET;
	int8_t x = next.x - current->x;
	int8_t y = next.y - current->y;

	NextStatus = next; // 下一个坐标信息更新

	if ((x > 1 || y > 1 || x < -1 || y < -1) || (x != 0 && y != 0))
	{
		print_info("Node Skipped!!\r\n");
		return;
	}

	// 获取需要转向
	finalDir = Get_TowardsByNode(*current, next);
	if (finalDir == DIR_NOTSET)
	{
		print_info("Same Node\r\n");
		return;
	}

	// 当前点非十字线时转向使用码盘转向
	if ((current->x % 2 == 0) || (current->y % 2 == 0))
	{
		Turn_ToDirection(&current->dir, finalDir, TurnOnce_EncoderMethod);
	}
	else
	{
		Turn_ToDirection(&current->dir, finalDir, TurnOnce_TrackMethod);
	}

	// 记录执行前是否为特殊地形
	uint8_t status = Special_RoadSection;

	// 若当前为特殊地形路段且已通过，跳过未行驶的路程
	// (status && Special_Road_Processed)

	// X轴为偶数的坐标
	if (next.x % 2 == 0)
	{
		// 横坐标0和6的为两侧车库，码盘值需要变小
		uint16_t encoderValue = ((next.x == 0) || (next.x == 6)) ? SidePark_Value : LongTrack_Value;

		if (Special_RoadSection)
		{
			encoderValue = ShortTrack_Value;
		}

		Track_ByEncoder(Track_Speed, encoderValue);

		while (Stop_Flag != FORBACKCOMPLETE)
		{
			// 长边，未到路中
			// 特殊地形在路中
			if (status && Special_Road_Processed)
			{
				MOVE(8);
				break;
			}
		}
		Stop();
		Submit_SpeedChanges();
	}
	// Y轴为偶数的坐标
	else if (next.y % 2 == 0)
	{
		Track_ByEncoder(Track_Speed, ShortTrack_Value);

		while (Stop_Flag != FORBACKCOMPLETE)
		{
			// 短边，未到路中
			// 特殊地形在下一十字路口上，需要跳节点
			// 注意：短边路中的特殊地形在上一节点到黑线之后出现
			if (status && Special_Road_Processed)
			{
				MOVE(8);
				skipNodes = 1;
				break;
			}
		}
		Stop();
		Submit_SpeedChanges();
	}
	// 前方十字路口
	else
	{
		// 循迹到十字路口
		Start_Tracking(Track_Speed);

		while (Stop_Flag != CROSSROAD)
		{
			// 十字路口前
			// 特殊地形地形在当前十字路口，直接跳出
			if (status && Special_Road_Processed)
			{
				MOVE(8);
				break;
			}
		}

		// 若未经过特殊地形，继续处理十字线过线部分路程
		if (!(status && Special_Road_Processed))
		{
			// 应对放在十字线后面的白卡
			Go_Ahead(Track_Speed, ToCrossroadCenter);
			Submit_SpeedChanges(); // 提交速度更改

			while (Stop_Flag != FORBACKCOMPLETE)
			{
				// RFID路段且未处理过
				if (RFID_RoadSection && (FOUND_RFID_CARD == false))
				{
					Get_Track();
					if (IS_All_WHITE())
					{
						FOUND_RFID_CARD = true; // 找到白卡
						Save_RunningStatus();   // 保存当前状态
						Stop();					// 暂停运行
						Submit_SpeedChanges();  // 提交速度更改
						TIM_Cmd(TIM5, ENABLE);  // 使能处理定时器
					}
				}
				// 特殊地形路段且未处理
				else if (status && (ENTER_SPECIAL_ROAD == false))
				{
					Get_Track();
					if (IS_All_WHITE())
					{
						ENTER_SPECIAL_ROAD = true;
						Save_RunningStatus();  // 保存当前状态
						Stop();				   // 暂停运行
						Submit_SpeedChanges(); // 提交速度更改
						TIM_Cmd(TIM5, ENABLE); // 使能处理定时器
					}
				}
				// 特殊路段已处理完毕
				if (status && Special_Road_Processed)
				{
					MOVE(8);
					break;
				}
			}
		}
		Stop();
		Submit_SpeedChanges();
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
	Update_MotorSpeed(0, 0);
	PidData_Clear();
	lastStopStamp = millis();
}

// 停止但不清空PID数据
void Stop_WithoutPIDClear(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	Update_MotorSpeed(0, 0);
	lastStopStamp = millis();
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
uint8_t turnLeftOrRight = DIR_NOTSET;
void Turn_ByTrack(Direction_t dir)
{
	if ((dir != DIR_RIGHT) && (dir != DIR_LEFT))
	{
		print_info("Turn Dir ERROR\r\n");
		return;
	}
	turnLeftOrRight = dir;

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

// 左右转，循迹线模式
void TurnOnce_TrackMethod(Direction_t dir)
{
	ExcuteAndWait(Turn_ByTrack(dir), Stop_Flag, TURNCOMPLETE);
}

// 左右转，码盘值模式
void TurnOnce_EncoderMethod(Direction_t dir)
{
	TURN((dir == DIR_LEFT) ? -90 : 90);
}

// 自动判断方向倒车入库
// 当前在车库入口时使用
void Reverse_Parcking(RouteNode_t *current, uint8_t targetGarage[3])
{
	uint8_t *currentStr = ReCoordinate_Convert(*current);
	Direction_t dir = Get_Towards(targetGarage, currentStr);

	// warning:码盘转向
	Turn_ToDirection(&current->dir, dir, TurnOnce_EncoderMethod);
	MOVE(-35);
}

// 全自动倒车入库
// 可做临时避让用
void Auto_ReverseParcking(RouteNode_t *current, uint8_t targetGarage[3], void (*taskAfterParcking)(void))
{
	RouteNode_t target = Coordinate_Convert(targetGarage);
	RouteNode_t *route = malloc(sizeof(RouteNode_t) * 12);
	uint8_t routeCount;

	A_Star_GetRouteBetweenNodes(*current, target, route, &routeCount);

	// 跳过第一个起始点和最后一个车库点
	for (uint8_t i = 1; i < routeCount - 1; i++)
	{
		NextStatus = route[i];
		Go_ToNextNode(current, route[i]);
	}
	free(route);

	Reverse_Parcking(current, targetGarage);

	// 入库完成后任务
	if (taskAfterParcking != NULL)
	{
		taskAfterParcking();
	}
}
