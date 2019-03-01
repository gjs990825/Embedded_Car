#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "cba.h"
#include "hardware.h"

void autorun(void);					 //ȫ�Զ���������
void zhuche_xunji(int speed);		 //����ѭ������
void go_ahead(int speed, u16 mapan); //ǰ����������
void back_off(int speed, u16 mapan); //���˺�������
void turn_left(int speed);			 //��ת��������
void turn_right(int speed);			 //��ת��������
void stop(void);					 //ֹͣ��������

/***************************************************************
** ���ܣ�     ����ȫ�Զ�����
** ������	  ��
** ����ֵ��   ��
** ˵����     ����ȫ�Զ�����
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
** ���ܣ�     ����ǰ��
** ������	  speed�����ͣ������ٶȣ�mapan��u16�ͣ�����ֵ
** ����ֵ��   ��
** ˵����     ����ֵԽ��ǰ������Խ��������ֵ���Ϊ65535 
****************************************************************/
void go_ahead(int speed, u16 mapan)
{
	Roadway_mp_syn(); //����ͬ��
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
** ���ܣ�     ��������
** ������	  speed�����ͣ������ٶȣ�mapan��u16�ͣ�����ֵ
** ����ֵ��   ��
** ˵����     ����ֵԽ��ǰ������Խ��������ֵ���Ϊ65535 
****************************************8************************/
void back_off(int speed, u16 mapan)
{
	Roadway_mp_syn(); //����ͬ��
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
** ���ܣ�     ������ת
** ������	  speed�����ͣ������ٶ�
** ����ֵ��   �� 
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
** ���ܣ�     ������ת
** ������	  speed�����ͣ������ٶ�
** ����ֵ��   �� 
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
** ���ܣ�     ����ֹͣ
** ������	  �޲���
** ����ֵ��   �� 
****************************************************************/
void stop(void)
{
	Roadway_Flag_clean(); //�����־λ״̬
}
/***************************************************************
** ���ܣ�   ����Ѱ��
** ������	  speed�����ͣ������ٶ�
** ����ֵ��   �� 
****************************************************************/
void zhuche_xunji(int speed)
{
	Go_Flag = 0;
	Stop_Flag = 0;
	Car_Spend = speed; //�����ٶ�
	Track_Flag = 1;
	// TIM_Cmd(TIM2, ENABLE);
}

void go_Test(uint16_t setMP)
{					  //���̿���ǰ��
					  //	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	Roadway_mp_syn(); //����ͬ��
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
{					  //���̿��ƺ���
					  //	OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	Roadway_mp_syn(); //����ͬ��
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
	Roadway_Flag_clean(); //�����־λ״̬
	Mp_Value = 0;
	h = 0;
	Send_UpMotor(0, 0);
}

void right_Test(uint16_t setNav) //�Ƕȿ�����ת
{
	Roadway_nav_syn(); //�Ƕ�ͬ��
	Roadway_mp_syn();  //����ͬ��
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

void left_Test(uint16_t setNav) //�Ƕȿ�����ת
{
	Roadway_nav_syn(); //�Ƕ�0ͬ��
	Roadway_mp_syn();  //����ͬ��
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
	Right180_Flag = 0; //ת�����
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
	Right180_Flag = 0; //ת�����
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
	Right180_Flag = 0; //ת�����
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
	Right180_Flag = 0; //ת�����
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
	Right180_Flag = 1; //ת�����
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
	Right180_Flag = 0; //ת�����
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
	Right180_Flag = 0; //ת�����
	Back_Flag = 0;
	Track_Flag = 1;
	temp_MP = 0;
	Track_Back_Flag = 1;
	temp_MP = setMP * 10;
	Car_Spend = Track_Speed;
	//	TIM_Cmd(TIM9,ENABLE);
}
