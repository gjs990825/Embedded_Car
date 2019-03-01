#ifndef __ROADWAY_H
#define __ROADWAY_H
//#include "stm32f4xx.h"
#include "sys.h"

#define Track_Speed 55   //循迹速度
#define Turn_Speed  85   //转弯速度

#define Turn_L45_MPval   375
#define Turn_R45_MPval   400
#define Turn_L90_MPval   860 
#define Turn_R90_MPval   880  

#define Turn_MP180  1960 //码盘控制180度转弯

#define LongTrack_Vale 110  //X轴循迹
#define ShortTrack_Vale 95  //Y轴循迹

void Roadway_Check(void);
void Roadway_Flag_clean(void);
void Roadway_mp_syn(void);
void Roadway_nav_syn(void);
uint16_t Roadway_mp_Get(void);

extern uint8_t wheel_L_Flag,wheel_R_Flag;
extern uint8_t Right45_Flag,Left45_Flag,Right90_Flag,Left90_Flag,Right180_Flag;
extern uint8_t wheel_Nav_Flag;
extern uint8_t Go_Flag,Back_Flag;
extern uint8_t Track_Flag;
extern uint8_t Back_Track_Falg;
extern int Car_Spend;
extern uint16_t temp_MP;
extern uint16_t temp_Nav;
extern uint8_t Stop_Flag;
extern uint8_t DX_Flag;
extern u8 gd1,gd2;
extern u8 h;
extern uint16_t Mp_Value;
extern uint8_t Track_Back_Flag;

void Control(int L_Spend,int R_Spend);

extern uint8_t Roadway_GoBack_Check(void);
void roadway_check_TimInit(uint16_t arr,uint16_t psc);
#endif


