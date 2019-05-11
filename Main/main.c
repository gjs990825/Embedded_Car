#include "hardware.h"

int main(void)
{
	uint32_t PowerCheckStamp = 0;
	uint32_t RFIDCheckStamp = 0;

	Hardware_Init();

	while (1)
	{
		KEY_Check(); //按键检测

		if (autoRunEnable)
		{
			Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStaus);
			autoRunEnable = 0;
			
			// 执行任务后进入死循环
			infinity_loop();
		}

		if (IsTimeOut(PowerCheckStamp, 200))
		{
			PowerCheckStamp = Get_GlobalTimeStamp();
			Power_Check();
		}

		if (IsTimeOut(RFIDCheckStamp, 300))
		{
			RFIDCheckStamp = Get_GlobalTimeStamp();
			if (Rc522_GetLinkFlag() == 0)
			{
				Readcard_Device_Init();
				MP_SPK = !MP_SPK;
			}
			else
			{
				MP_SPK = 0;
				LED4 = !LED4;
				Rc522_LinkTest();
			}
		}
	}
}

// 参数错误的处理
void assert_failed(uint8_t *file, uint32_t line)
{
	print_info("ERR: %s, %d\r\n", file, line);
	infinity_loop();
}
