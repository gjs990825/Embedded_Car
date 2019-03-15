#include "hardware.h"

static uint32_t Power_check_times; //�����������
// static uint32_t LED_twinkle_times;		//LED��˸����
static uint32_t WIFI_Upload_data_times; //ͨ��Wifi�ϴ���������
static uint32_t RFID_Init_Check_times;

int main(void)
{
	// uint16_t Light_Value = 0;	 //��ǿ��ֵ
	// uint16_t CodedDisk_Value = 0; //����
	// uint16_t Nav_Value = 0;		  //�Ƕ�

	Hardware_Init(); //Ӳ����ʼ��

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
		KEY_Check(); //�������

		// ���ڷ����ж���
		// Can_WifiRx_Check();
		// Can_ZigBeeRx_Check();

		// if(gt_get_sub(LED_twinkle_times) == 0)
		// {
		// 	LED_twinkle_times =  gt_get() + 50;			//LED4״̬ȡ��
		// 	LED4 = !LED4;
		// }
		if (autoRunEnable)
		{
			Auto_Run();
			autoRunEnable = 0;
		}

		if (gt_get_sub(Power_check_times) == 0)
		{
			Power_check_times = gt_get() + 200; //��ص������
			Power_Check();
		}

		if (gt_get_sub(RFID_Init_Check_times) == 0)
		{
			RFID_Init_Check_times = gt_get() + 200; //RFID��ʼ�����
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
			// 	Principal_Tab[2] = Stop_Flag;			  //����״̬
			// 	Principal_Tab[3] = Get_tba_phsis_value(); //����״ֵ̬����

			// 	Ultrasonic_Ranging(); //����������
			// 	Principal_Tab[4] = distance % 256;
			// 	Principal_Tab[5] = distance / 256;

			// 	Light_Value = Get_Bh_Value();		  //��ǿ�ȴ�����
			// 	Principal_Tab[6] = Light_Value % 256; //��������
			// 	Principal_Tab[7] = Light_Value / 256;

			// 	CodedDisk_Value = CanHost_Mp; //����
			// 	Principal_Tab[8] = CodedDisk_Value % 256;
			// 	Principal_Tab[9] = CodedDisk_Value / 256;

			// 	Nav_Value = CanHost_Navig; //�Ƕ�
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
