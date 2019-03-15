#include "hardware.h"

// ��ʼ��Ӳ��
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); //�жϷ���
	delay_init(168);

	Tba_Init();						 // ������ʼ��
	Infrared_Init();				 // �����ʼ��
	Cba_Init();						 // ���İ��ʼ��
	Ultrasonic_Init();				 // ��������ʼ��
	Hard_Can_Init();				 // CAN���߳�ʼ��
	BH1750_Configure();				 // BH1750��ʼ������
	SYN7318_Init();					 // ����ʶ���ʼ��
	Electricity_Init();				 // ��������ʼ��
	UartA72_Init();					 // ��A72������Ĵ���
	Can_check_Init(83, 999);		 // CAN���߶�ʱ����ʼ�� (83, 7)??????
	Roadway_CheckTimInit(167, 1999); // ·�����
	Timer_Init(167, 999);			 // ��������ͨѶʱ��֡
	Readcard_Device_Init();			 // RFID��ʼ��

	my_mem_init(SRAMIN);		// ��ʼ���ڲ��ڴ��


	
	DebugTimer_Init(16799, 1999); // ����(�׿����)
	CanTimer_Init(16799, 199); // 20ms CAN���ݼ��
	

	print_info("\r\nSystem running...\r\n");
}
