#ifndef __ROADWAY_H
#define __ROADWAY_H
//#include "stm32f4xx.h"
#include "sys.h"

#define Track_Speed 55 //循迹速度
#define Turn_Speed 85  //转弯速度

#define Turn_L45_MPval 375
#define Turn_R45_MPval 400
#define Turn_L90_MPval 860
#define Turn_R90_MPval 880

#define Turn_MP180 1960 //码盘控制180度转弯

#define LongTrack_Vale 110 //X轴循迹
#define ShortTrack_Vale 95 //Y轴循迹

void Roadway_Check(void);
void Roadway_Flag_clean(void);
void Roadway_mp_syn(void);
void Roadway_nav_syn(void);
uint16_t Roadway_mp_Get(void);


// 左右转向
extern uint8_t wheel_L_Flag, wheel_R_Flag;
// 转向角度
extern uint8_t Right45_Flag, Left45_Flag, Right90_Flag, Left90_Flag, Right180_Flag;
// 转向
extern uint8_t wheel_Nav_Flag;
// 前进后退
extern uint8_t Go_Flag, Back_Flag;
// 循迹
extern uint8_t Track_Flag;
// 倒退循迹
extern uint8_t Track_Back_Flag;
extern uint8_t Line_Flag;

// 运行状态
extern uint8_t Stop_Flag;
// 左右速度
extern int LSpeed, RSpeed;
// 设定车速
extern int Car_Spend;

// 码盘值
extern uint16_t temp_MP;
extern uint16_t temp_Nav;

extern uint8_t Wheel_flag;

// 循迹结果
extern u8 gd1, gd2;
// 调整车身
extern u8 h;
// 循迹模式
extern u8 track_mode;
// 当前码盘值
extern uint16_t Mp_Value;


/////
void Get_Track(void);
void Get_DirectionWights(void);
void TRACK_LINE(void);

void Control(int L_Spend, int R_Spend);

extern uint8_t Roadway_GoBack_Check(void);
void roadway_check_TimInit(uint16_t arr, uint16_t psc);
#endif
