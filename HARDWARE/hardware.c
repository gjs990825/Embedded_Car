#include "hardware.h"

// 初始化时钟、外设、定时器...
void Hardware_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断优先级分组
	delay_init(168);

	Hard_Can_Init();	// CAN 总线初始化
	Tba_Init();			// 任务板初始化
	Infrared_Init();	// 红外初始化
	Cba_Init();			// 核心板初始化
	Ultrasonic_Init();  // 超声波初始化
	BH1750_Configure(); // BH1750 初始化
	SYN7318_Init();		// 语音识别初始化
	Electricity_Init(); // 电量检测初始化
	UartA72_Init();		// 至 A72 开发板的串口

	// 默认值(83, 7)频率过高，降低速度后目前没有发现问题
	Can_check_Init(83, 999); // CAN 总线定时器初始化

	Roadway_CheckTimInit(167, 1999); // 路况检测
	Timer_Init(167, 999);			 // 全局时间
	Readcard_Device_Init();			 // RC522 初始化

	my_mem_init(SRAMIN); // 初始化内部内存池
	// my_mem_init(SRAMEX);
	// 外部内存池未配置接口，暂不可用

	CanTimer_Init(167, 1999);	 // CAN 数据检查
	DebugTimer_Init(16799, 1999); // 调试（白卡检测）
	DebugPin_Init();			  // 初始化调试用引脚
	Update_MotorSpeed(0, 0);	  // 电机速度置零

	print_info("\r\nSystem running...\r\n");
}
