#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"


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

#endif // _MOVEMENT_H_
