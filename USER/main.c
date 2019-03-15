#include "hardware.h"

static uint32_t Power_check_times; //电量检测周期
// static uint32_t LED_twinkle_times;		//LED闪烁周期
static uint32_t WIFI_Upload_data_times; //通过Wifi上传数据周期
static uint32_t RFID_Init_Check_times;

int main(void)
{
	// uint16_t Light_Value = 0;	 //光强度值
	// uint16_t CodedDisk_Value = 0; //码盘
	// uint16_t Nav_Value = 0;		  //角度

	Hardware_Init(); //硬件初始化

	// LED_twinkle_times = gt_get() + 50;
	Power_check_times = gt_get() + 200;
	WIFI_Upload_data_times = gt_get() + 200;
	RFID_Init_Check_times = gt_get() + 200;

	Principal_Tab[0] = 0x55;
	Principal_Tab[1] = 0xAA;

	Follower_Tab[0] = 0x55;
	Follower_Tab[1] = 0x02;

	Send_UpMotor(0, 0);

	Send_DataToUsart("USART TEST\r\n", 13);

	while (1)
	{
		KEY_Check(); //按键检测

		// 现在放在中断中
		// Can_WifiRx_Check();
		// Can_ZigBeeRx_Check();

		// if(gt_get_sub(LED_twinkle_times) == 0)
		// {
		// 	LED_twinkle_times =  gt_get() + 50;			//LED4状态取反
		// 	LED4 = !LED4;
		// }
		if (autoRunEnable)
		{
			Auto_Run();
			autoRunEnable = 0;
		}

		if (gt_get_sub(Power_check_times) == 0)
		{
			Power_check_times = gt_get() + 200; //电池电量检测
			Power_Check();
		}

		if (gt_get_sub(RFID_Init_Check_times) == 0)
		{
			RFID_Init_Check_times = gt_get() + 200; //RFID初始化检测
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

		if (gt_get_sub(WIFI_Upload_data_times) == 0)
		{
			// WIFI_Upload_data_times = gt_get() + 200;

			// if (Host_AGV_Return_Flag == RESET)
			// {
			// 	Principal_Tab[2] = Stop_Flag;			  //运行状态
			// 	Principal_Tab[3] = Get_tba_phsis_value(); //光敏状态值返回

			// 	Ultrasonic_Ranging(); //超声波数据
			// 	Principal_Tab[4] = distance % 256;
			// 	Principal_Tab[5] = distance / 256;

			// 	Light_Value = Get_Bh_Value();		  //光强度传感器
			// 	Principal_Tab[6] = Light_Value % 256; //光照数据
			// 	Principal_Tab[7] = Light_Value / 256;

			// 	CodedDisk_Value = CanHost_Mp; //码盘
			// 	Principal_Tab[8] = CodedDisk_Value % 256;
			// 	Principal_Tab[9] = CodedDisk_Value / 256;

			// 	Nav_Value = CanHost_Navig; //角度
			// 	Principal_Tab[10] = Nav_Value % 256;
			// 	Principal_Tab[11] = Nav_Value / 256;

			// 	Send_WifiData_To_Fifo(Principal_Tab, 12);
			// 	UartA72_TxClear();
			// 	UartA72_TxAddStr(Principal_Tab, 12);
			// 	UartA72_TxStart();
			// }
			// else if ((Host_AGV_Return_Flag == SET) && (AGV_data_Falg == SET))
			// {

			// 	UartA72_TxClear();
			// 	UartA72_TxAddStr(Follower_Tab, 50);
			// 	UartA72_TxStart();
			// 	Send_WifiData_To_Fifo(Follower_Tab, 50);
			// 	AGV_data_Falg = 0;
			// }
		}
	}
}
