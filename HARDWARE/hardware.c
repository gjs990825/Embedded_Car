#include "hardware.h"

// 初始化硬件
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); //中断分组
	delay_init(168);

	Tba_Init();						 // 任务板初始化
	Infrared_Init();				 // 红外初始化
	Cba_Init();						 // 核心板初始化
	Ultrasonic_Init();				 // 超声波初始化
	Hard_Can_Init();				 // CAN总线初始化
	BH1750_Configure();				 // BH1750初始化配置
	SYN7318_Init();					 // 语音识别初始化
	Electricity_Init();				 // 电量检测初始化
	UartA72_Init();					 // 至A72开发板的串口
	Can_check_Init(83, 999);		 // CAN总线定时器初始化 (83, 7)??????
	Roadway_CheckTimInit(167, 1999); // 路况检测
	Timer_Init(167, 999);			 // 串行数据通讯时间帧
	Readcard_Device_Init();			 // RFID初始化

	my_mem_init(SRAMIN);		// 初始化内部内存池


	
	DebugTimer_Init(16799, 1999); // 调试(白卡检测)
	CanTimer_Init(16799, 199); // 20ms CAN数据检查
	

	print_info("\r\nSystem running...\r\n");
}
