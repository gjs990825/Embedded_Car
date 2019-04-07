#include "hardware.h"

// ��ʼ��Ӳ��
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); // �ж����ȼ�����
	delay_init(168);

	Tba_Init();			// ������ʼ��
	Infrared_Init();	// �����ʼ��
	Cba_Init();			// ���İ��ʼ��
	Ultrasonic_Init();  // ��������ʼ��
	Hard_Can_Init();	// CAN ���߳�ʼ��
	BH1750_Configure(); // BH1750 ��ʼ��
	SYN7318_Init();		// ����ʶ���ʼ��
	Electricity_Init(); // ��������ʼ��
	UartA72_Init();		// �� A72 ������Ĵ���

	// Ĭ��ֵ(83, 7)Ƶ�ʹ��ߣ������ٶȺ�Ŀǰû�з�������
	Can_check_Init(83, 999); // CAN ���߶�ʱ����ʼ��

	Roadway_CheckTimInit(167, 1999); // ·�����
	Timer_Init(167, 999);			 // ��������ͨѶʱ��֡
	Readcard_Device_Init();			 // RC522 ��ʼ��

	my_mem_init(SRAMIN); // ��ʼ���ڲ��ڴ��
	// my_mem_init(SRAMEX); // ��ʼ���ⲿ�ڴ�أ�δ���ã��ݲ����ã�

	CanTimer_Init(16799, 199);	  // CAN ���ݼ��
	DebugTimer_Init(16799, 1999); // ���ԣ��׿���⣩
	DebugPin_Init();			  // ��ʼ������������

	print_info("\r\nSystem running...\r\n");
}
