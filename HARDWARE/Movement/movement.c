#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"

void autorun(void);					 //全自动函数声明
void zhuche_xunji(int speed);		 //主车循迹函数
void go_ahead(int speed, u16 mapan); //前进函数声明
void back_off(int speed, u16 mapan); //后退函数声明
void turn_left(int speed);			 //左转函数声明
void turn_right(int speed);			 //右转函数声明
void stop(void);					 //停止函数声明

/***************************************************************
** 功能：     主车全自动程序
** 参数：	  无
** 返回值：   无
** 说明：     主车全自动函数
****************************************************************/
void autorun(void)
{

	zhuche_xunji(50);
	while (Stop_Flag != CROSSROAD)
		;
	stop_Test();
	Beep(1);
	
	go_ahead(30, 300);
	while (Stop_Flag != FORBACKCOMPLETE)
		;
	stop_Test();
	Beep(1);

	zhuche_xunji(70);
	while (Stop_Flag != CROSSROAD)
		;
	stop_Test();
	Beep(1);
	// go_ahead(80, 400);
	// while (Stop_Flag != 3)
	// {
	// 	;
	// }
	// zhuche_xunji(60);
	// while (Stop_Flag != 1)
	// {
	// 	;
	// }
}

/***************************************************************
** 功能：     主车前进
** 参数：	  speed，整型，主车速度；mapan，u16型，码盘值
** 返回值：   无
** 说明：     码盘值越大，前进距离越长，码盘值最大为65535 
****************************************************************/
void go_ahead(int speed, u16 mapan)
{
	Roadway_mp_syn(); //码盘同步
	Stop_Flag = 0;
	Go_Flag = 1;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Back_Flag = 0;
	Track_Flag = 0;
	temp_MP = mapan;
	Car_Spend = speed;
	Control(Car_Spend, Car_Spend);
}
/***************************************************************
** 功能：     主车后退
** 参数：	  speed，整型，主车速度；mapan，u16型，码盘值
** 返回值：   无
** 说明：     码盘值越大，前进距离越长，码盘值最大为65535 
****************************************8************************/
void back_off(int speed, u16 mapan)
{
	Roadway_mp_syn(); //码盘同步
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Back_Flag = 1;
	Track_Flag = 0;
	temp_MP = mapan;
	Car_Spend = speed;
	Control(-Car_Spend, -Car_Spend);
}

/***************************************************************
** 功能：     主车左转
** 参数：	  speed，整型，主车速度
** 返回值：   无 
****************************************************************/
void turn_left(int speed)
{
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 1;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = speed;
	Control(-Car_Spend, Car_Spend);
}

/***************************************************************
** 功能：     主车右转
** 参数：	  speed，整型，主车速度
** 返回值：   无 
****************************************************************/
void turn_right(int speed)
{
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 1;
	wheel_Nav_Flag = 0;
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = speed;
	Control(Car_Spend, -Car_Spend);
}

/***************************************************************
** 功能：     主车停止
** 参数：	  无参数
** 返回值：   无 
****************************************************************/
void stop(void)
{
	Roadway_Flag_clean(); //清除标志位状态
}
/***************************************************************
** 功能：   主车寻迹
** 参数：	  speed，整型，主车速度
** 返回值：   无 
****************************************************************/
void zhuche_xunji(int speed)
{
	Go_Flag = 0;
	Stop_Flag = 0;
	Car_Spend = speed; //主车速度
	Track_Flag = 1;
	// TIM_Cmd(TIM2, ENABLE);
}

void go_Test(uint16_t setMP)
{					  //码盘控制前进
					  //	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	Roadway_mp_syn(); //码盘同步
	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 1;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Track_Speed;
	temp_MP = setMP * 10;
	Control(Car_Spend, Car_Spend);
}

void back_Test(uint16_t setMP)
{					  //码盘控制后退
					  //	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	Roadway_mp_syn(); //码盘同步
					  //	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Back_Flag = 1;
	Track_Flag = 0;
	temp_MP = setMP * 10;
	Car_Spend = Track_Speed;
	Control(-Car_Spend, -Car_Spend);
}

void stop_Test(void)
{
	Roadway_Flag_clean(); //清除标志位状态
	Mp_Value = 0;
	h = 0;
	Send_UpMotor(0, 0);
}

void right_Test(uint16_t setNav) //角度控制右转
{
	Roadway_nav_syn(); //角度同步
	Roadway_mp_syn();  //码盘同步
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 1;
	Back_Flag = 0;
	Track_Flag = 0;
	temp_Nav = setNav;
	Car_Spend = Turn_Speed;
	Send_UpMotor(Car_Spend, -Car_Spend);
	//	TIM_Cmd(TIM9,ENABLE);
}

void left_Test(uint16_t setNav) //角度控制左转
{
	Roadway_nav_syn(); //角度0同步
	Roadway_mp_syn();  //码盘同步
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 1;
	Back_Flag = 0;
	Track_Flag = 0;
	temp_Nav = setNav;
	Car_Spend = Turn_Speed;
	Send_UpMotor(-Car_Spend, Car_Spend);
	//	TIM_Cmd(TIM9,ENABLE);
}

void left45_Test(void)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 1, Right90_Flag = 0, Left90_Flag = 0;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Turn_Speed;
	Control(-Car_Spend, Car_Spend);
	//  TIM_Cmd(TIM9,ENABLE);
}

void left90_Test(void)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 1;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Turn_Speed;
	Control(-Car_Spend, Car_Spend);
	//  TIM_Cmd(TIM9,ENABLE);
}

void right45_Test(void)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 1, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Turn_Speed;
	Control(Car_Spend, -Car_Spend);
	//	TIM_Cmd(TIM9,ENABLE);
}

void right90_Test(void)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 1, Left90_Flag = 0;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Turn_Speed;
	Control(Car_Spend, -Car_Spend);
	//	TIM_Cmd(TIM9,ENABLE);
}

void right180_Test(void)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
	Right180_Flag = 1; //转弯控制
	Back_Flag = 0;
	Track_Flag = 0;
	Car_Spend = Turn_Speed;
	Control(Car_Spend, -Car_Spend);
	//  TIM_Cmd(TIM9,ENABLE);
}

void Track_MP(uint16_t setMP)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 1;
	temp_MP = 0;
	temp_MP = setMP * 10;
	Car_Spend = Track_Speed;
	//	TIM_Cmd(TIM9,ENABLE);
}

void Track_Back(uint16_t setMP)
{
	Roadway_mp_syn();
	//	Mp_Value = Roadway_mp_Get();
	Stop_Flag = 0;
	Go_Flag = 0;
	wheel_L_Flag = 0;
	wheel_R_Flag = 0;
	wheel_Nav_Flag = 0;
	Right45_Flag = 0, Left45_Flag = 0, Right90_Flag = 0, Left90_Flag = 0;
	Right180_Flag = 0; //转弯控制
	Back_Flag = 0;
	Track_Flag = 1;
	temp_MP = 0;
	Track_Back_Flag = 1;
	temp_MP = setMP * 10;
	Car_Spend = Track_Speed;
	//	TIM_Cmd(TIM9,ENABLE);
}
