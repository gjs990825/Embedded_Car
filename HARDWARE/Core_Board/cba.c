#include "stm32f4xx.h"
#include "infrared.h"
#include "delay.h"
#include "cba.h"

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
	for(uint8_t i = 0; i < times; i++)
	{
		MP_SPK = 1;
		delay_ms(200);
		MP_SPK = 0;
		delay_ms(200);
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
			LED1 = !LED1;
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
			LED2 = !LED2;
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
			LED3 = !LED3;
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
