#ifndef __ROADWAY_H
#define __ROADWAY_H
//#include "stm32f4xx.h"
#include "sys.h"

#define Track_Speed 45 // 55 //循迹速度
#define Turn_Speed 85  //转弯速度

#define Turn_L45_MPval 375
#define Turn_R45_MPval 400
#define Turn_L90_MPval 860
#define Turn_R90_MPval 880

#define Turn_MP180 1800 //码盘控制180度转弯

#define LongTrack_Value 1100 //X轴循迹
#define ShortTrack_Value 950 //Y轴循迹

#define ToCrossroadCenter 420 // 到十字路口中心

#define TrackMode_NONE 0
#define TrackMode_NORMAL 1
#define TrackMode_ENCODER 2
#define TrackMode_BACK 4

void Roadway_Check(void);
void Roadway_Flag_clean(void);
void Roadway_mp_syn(void);
void Roadway_nav_syn(void);
uint16_t Roadway_mp_Get(void);

typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_LEFT90,
    ENCODER_LEFT45,
    ENCODER_RIGHT90,
    ENCODER_RIGHT45,
    ENCODER_RIGHT180
} Moving_ByEncoder_t;

// 循迹信息
extern int8_t Q7[7], H8[8];
// 白色个数
extern uint8_t NumberOfWhite;
// 方向权重
extern int DirectionWights;
// 循迹模式
extern uint8_t Track_Mode;
// 定值前后和转向
extern Moving_ByEncoder_t Moving_ByEncoder;

extern uint8_t Stop_Flag;
extern int Car_Speed;
extern uint16_t temp_MP;


void Get_Track(void);
void Get_DirectionWights(void);
void TRACK_LINE(void);

void Control(int LSpeed, int RSpeed);

extern uint8_t Roadway_GoBack_Check(void);
void Roadway_CheckTimInit(uint16_t arr, uint16_t psc);

#endif
