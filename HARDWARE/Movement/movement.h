#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"


/// Gao added
void Go_ToNextNode(RouteNode next);
void Start_Tracking(int speed);	

void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_Left45(void);
void Turn_Left90(void);
void Turn_Right45(void);
void Turn_Right90(void);
void Turn_Right180(void);

void Track_ByEncoder(int speed, uint16_t setMP);




#define _USE_OLD_MOVING_CONTROL_ 0





void autorun(void);

#if _USE_OLD_MOVING_CONTROL_

void go_Test(uint16_t setMP);    
void back_Test(uint16_t setMP);
void stop_Test(void);

void right_Test(uint16_t setNav); //电子罗盘控制转弯角度
void left_Test(uint16_t setNav);

void left45_Test(void);   //码盘控制带角度转弯
void left90_Test(void);
void right45_Test(void);
void right90_Test(void);
void right180_Test(void);

void Track_MP(uint16_t setMP); //带码盘的循迹

void Track_Back(uint16_t setMP);// 后退循迹

void track_go(u16 setMP);
void track_back(u16 setMP);

#endif // _USE_OLD_MOVING_CONTROL_

#endif // _MOVEMENT_H_
