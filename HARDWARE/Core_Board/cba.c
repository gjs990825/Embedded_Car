#include "stm32f4xx.h"
#include "infrared.h"
#include "delay.h"
#include "cba.h"

// 按键设置启动程序

#include "infrared.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"
#include "voice.h"
#include "data_interaction.h"
#include "debug.h"
#include "bh1750.h"

// Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStatus);
// Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStatus);

#define KEY_DEFAULT 0
#define KEY_DATA_INTERACTION 1
#define KEY_AGV_TEST 2
#define KEY_RFID_TEST 3
#define KEY_TASK_BOARD_TEST 4
#define KEY_TEMP 5

// 配置按键调试方案
#define KEY_CONFIGURATION KEY_DATA_INTERACTION

#if (KEY_CONFIGURATION == KEY_DEFAULT)

// 默认配置
#define Action_S1() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStatus);
#define Action_S2() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStatus);
#define Action_S3() Infrared_Send_A(Infrared_AlarmON)
#define Action_S4() print_info("light:%d\r\n", BH1750_GetAverage(10))

#elif (KEY_CONFIGURATION == KEY_DATA_INTERACTION)

// 数据交互配置

#define Action_S1() print_info("Plate:%s\r\n", Get_PlateNumber(TFT_A))
#define Action_S2() print_info("QRCode:%s\r\n", Get_QRCode(DataRequest_QRCode1, 0))
#define Action_S3() print_info("Shape:%d\r\n", Get_ShapeNumber(TFT_A, Shape_Triangle))
#define Action_S4() print_info("AllColor:%d\r\n", Get_AllColorCount(TFT_A))

// #define Action_S1() print_info("PlateNumber:%s\r\n", Get_PlateNumber(TFT_A))
// #define Action_S2() print_info("QRCode:%s\r\n", Get_QRCode(DataRequest_QRCode1, 2))
// #define Action_S3() print_info("TrafficLight:%d\r\n", Get_TrafficLight(TrafficLight_A))
// #define Action_S4() print_info("ShapeNumber:%d\r\n", Get_ShapeNumber(TFT_A, Shape_Circle))

// #define Action_S1() print_info("ColorNumber:%d\r\n", Get_ColorNumber(TFT_A, Color_Cyan))
// #define Action_S2() print_info("ShapeColorNumber:%d\r\n", Get_ShapeColorNumber(TFT_A, Shape_Rectangle, Color_Red))
// #define Action_S3() print_info("RFIDInfo:%s\r\n", Get_RFIDInfo(2))
// #define Action_S4() print_info("TFTInfo:%s\r\n", Get_TFTInfo(TFT_A))

// #define Action_S1() print_info("AllColorCount:%d\r\n", Get_AllColorCount(TFT_A))
// #define Action_S2() print_info("AllShapeCount:%d\r\n", Get_AllShapeCount(TFT_B))
// #define Action_S3() (void)0
// #define Action_S4() (void)0

#elif (KEY_CONFIGURATION == KEY_AGV_TEST)

// 从车测试配置
#define Action_S1() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStatus);
#define Action_S2() AGV_SetTaskID(1, 0)
#define Action_S3() AGV_SetRoute("B7B6D6D4G4")
#define Action_S4()                                  \
	CurrentStatus.x = 5;                             \
	CurrentStatus.y = 5;                             \
	CurrentStatus.dir = DIR_UP;                      \
	DataToAGV_t AGVData;                             \
	taskCoord_t taskCoord[2];                        \
	taskCoord[0].coord = "B2";                       \
	taskCoord[0].taskID = AGVPresetTask_Streetlight; \
	taskCoord[1].coord = "D6";                       \
	taskCoord[1].taskID = 0;                         \
	AGVData.alarmData = Infrared_AlarmON;            \
	AGVData.avoidGarage = "G2";                      \
	AGVData.barrierGateCoord = "F3";                 \
	AGVData.currentCoord = "F5";                     \
	AGVData.direction = DIR_RIGHT;                   \
	AGVData.routeInfo = "F2B2B6D6";                  \
	AGVData.streetLightLevel = 3;                    \
	AGVData.taskCoord = taskCoord;                   \
	AGVData.tasknumber = 2;                          \
	AGV_Task(AGVData);

#elif (KEY_CONFIGURATION == KEY_RFID_TEST)

// 白卡调试配置
#define Action_S1() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStatus);
#define Action_S2() Read_RFID(&RFID1)
#define Action_S3() Read_RFID(&RFID2)
#define Action_S4() Read_RFID(&RFID3)

#elif (KEY_CONFIGURATION == KEY_TASK_BOARD_TEST)

// 任务板调试配置
#define Action_S1() Infrared_Send_A(Infrared_AlarmON)
#define Action_S2() print_info("Diatance:%d\r\n", Ultrasonic_Task(20))
#define Action_S3() print_info("light:%d\r\n", BH1750_GetAverage(10))
#define Action_S4() Start_VoiceCommandRecognition(3)

#elif (KEY_CONFIGURATION == KEY_TEMP)

// 临时配置
#define Action_S1() (void)0
#define Action_S2() (void)0
#define Action_S3() (void)0
#define Action_S4() (void)0

#endif

// 核心板初始化（KEY/LED/BEEP）
void Cba_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH, ENABLE);

	// GPIOI4\5\6\7----KEY
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //通用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOI, &GPIO_InitStructure);

	// GPIOH12\13\14\15-----LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //通用输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	// GPIOH5 ---- MP_SPK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //通用输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
}

// 蜂鸣N次
void Beep(uint8_t Ntimes)
{
	for (uint8_t i = 0; i < Ntimes; i++)
	{
		MP_SPK = 1;
		delay_ms(70);
		MP_SPK = 0;
		delay_ms(30);
	}
}

// 按键检测
void KEY_Check(void)
{
	if (S1 == 0)
	{
		delay_ms(10);
		if (S1 == 0)
		{
			while (!S1)
				;
			Action_S1();
		}
	}
	if (S2 == 0)
	{
		delay_ms(10);
		if (S2 == 0)
		{
			while (!S2)
				;
			Action_S2();
		}
	}
	if (S3 == 0)
	{
		delay_ms(10);
		if (S3 == 0)
		{
			while (!S3)
				;
			Action_S3();
		}
	}
	if (S4 == 0)
	{
		delay_ms(10);
		if (S4 == 0)
		{
			while (!S4)
				;
			Action_S4();
		}
	}
}
