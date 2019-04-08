#include "stm32f4xx.h"
#include "infrared.h"
#include "delay.h"
#include "cba.h"

// Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
// Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);

// // 默认配置
// #define Action_S1() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
// #define Action_S2() Start_VoiceCommandRecognition(3)
// #define Action_S3() Test_RFID(5)
// #define Action_S4() print_info("light:%d\r\n", BH1750_GetAverage(10))

// // 数据交互配置
// #define Action_S1() print_info("Plate:%s\r\n", Get_PlateNumber())
// #define Action_S2() print_info("QRCOde:%s\r\n", Get_QRCode(DataRequest_QRCode1, 0))
// #define Action_S3() print_info("Shape:%d\r\n", Get_ShapeNumber(Shape_Triangle))
// #define Action_S4() print_info("AllColor:%d\r\n", Get_AllColorCount())

// // 从车测试配置
// #define Action_S1() AGV_SetTowards(DIR_UP)
// #define Action_S2() AGV_SetTaskID(1, 0)
// #define Action_S3() AGV_SetRoute("G4F4F6D6D4D2F2G2")
// #define Action_S4() AGV_SendInfraredData(Infrared_AlarmON)

// // 白卡调试配置
// #define Action_S1() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);
// #define Action_S2() Test_RFID(6)
// #define Action_S3() Test_RFID(5)
// #define Action_S4() Test_RFID(4)

// // 任务板调试配置
#define Action_S1() Infrared_Send_A(Infrared_AlarmON)
#define Action_S2() AGV_SetTaskID(1, 0)
#define Action_S3() AGV_SendInfraredData(Infrared_AlarmON)
#define Action_S4() Auto_Run(RFID_TestRoute, RFID_TESTROUTE_NUMBER, &CurrentStaus);

// // // 任务板调试配置
// #define Action_S1() Infrared_Send_A(Infrared_AlarmON)
// #define Action_S2() print_info("Diatance:%d\r\n", Ultrasonic_Task(20))
// #define Action_S3() print_info("light:%d\r\n", BH1750_GetAverage(10))
// #define Action_S4() Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
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
			// LED1 = !LED1;
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
			// LED2 = !LED2;
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
			// LED3 = !LED3;
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
