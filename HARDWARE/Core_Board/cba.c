#include "stm32f4xx.h"
#include "infrared.h"
#include "delay.h"
#include "cba.h"

// 按键设置启动程序

#include "infrared.h"
#include "data_base.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "data_base.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"
#include "voice.h"
#include "data_from_host.h"
#include "debug.h"
#include "bh1750.h"

// Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
// Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);

#define KEY_DEFAULT 0
#define KEY_DATA_INTERACTION 1
#define KEY_AGV_TEST 2
#define KEY_RFID_TEST 3
#define KEY_TASK_BOARD_TEST 4
#define KEY_TEMP 5

// 配置按键调试方案
#define KEY_CONFIGURATION KEY_TEMP

#if (KEY_CONFIGURATION == KEY_DEFAULT)

// 默认配置
#define Action_S1() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
#define Action_S2() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);
#define Action_S3() Test_RFID(5)
#define Action_S4() print_info("light:%d\r\n", BH1750_GetAverage(10))

#elif (KEY_CONFIGURATION == KEY_DATA_INTERACTION)

// 数据交互配置
#define Action_S1() print_info("Plate:%s\r\n", Get_PlateNumber())
#define Action_S2() print_info("QRCOde:%s\r\n", Get_QRCode(DataRequest_QRCode1, 0))
#define Action_S3() print_info("Shape:%d\r\n", Get_ShapeNumber(Shape_Triangle))
#define Action_S4() print_info("AllColor:%d\r\n", Get_AllColorCount())

#elif (KEY_CONFIGURATION == KEY_AGV_TEST)

// 从车测试配置
#define Action_S1() AGV_SetTowards(DIR_UP)
#define Action_S2() AGV_SetTaskID(1, 0)
#define Action_S3() AGV_SetRoute("B7B6D6D4G4")
#define Action_S4()                             \
	do                                          \
	{                                           \
		AGV_SendInfraredData(Infrared_AlarmON); \
		delay_ms(700);                          \
		AGV_SendInfraredData(Infrared_AlarmON); \
		delay_ms(700);                          \
		AGV_SetTaskID(2, 0);                    \
		delay_ms(700);                          \
		AGV_SetTaskID(5, 1);                    \
		delay_ms(700);                          \
		AGV_SetTaskID(2, 0);                    \
		delay_ms(700);                          \
		AGV_SetTaskID(5, 1);                    \
		delay_ms(700);                          \
		AGV_SetRoute("G4F4D4D2B2B1\0\0\0");     \
		delay_ms(700);                          \
		AGV_SetTowards(DIR_LEFT);               \
		delay_ms(700);                          \
		AGV_SetTowards(DIR_LEFT);               \
		delay_ms(700);                          \
		AGV_Start();                            \
		delay_ms(700);                          \
		AGV_Start();                            \
	} while (0)

#elif (KEY_CONFIGURATION == KEY_RFID_TEST)

// 白卡调试配置
#define Action_S1() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);
#define Action_S2() Test_RFID(6)
#define Action_S3() Test_RFID(5)
#define Action_S4() Test_RFID(4)

#elif (KEY_CONFIGURATION == KEY_TASK_BOARD_TEST)

// 任务板调试配置
#define Action_S1() Infrared_Send_A(Infrared_AlarmON)
#define Action_S2() print_info("Diatance:%d\r\n", Ultrasonic_Task(20))
#define Action_S3() print_info("light:%d\r\n", BH1750_GetAverage(10))
#define Action_S4() Start_VoiceCommandRecognition(3)

#elif (KEY_CONFIGURATION == KEY_TEMP)

// 临时配置
#define Action_S1() Turn_ByTrack(DIR_RIGHT)
#define Action_S2() Turn_ByTrack(DIR_LEFT)
#define Action_S3() Start_Tracking(Track_Speed)
#define Action_S4() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);

#endif

// 核心板初始化（KEY/LED/BEEP）
void Cba_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH, ENABLE);

	//GPIOI4\5\6\7----KEY
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; //通用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOI, &GPIO_InitStructure);

	//GPIOH12\13\14\15-----LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //通用输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	//GPIOH5 ---- MP_SPK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //通用输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
}

void Beep(uint8_t times)
{
	for (uint8_t i = 0; i < times; i++)
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
