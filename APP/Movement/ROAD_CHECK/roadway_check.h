#ifndef __ROADWAY_H
#define __ROADWAY_H
//#include "stm32f4xx.h"
#include "sys.h"

// 速度常量↓

// 循迹速度
#define Track_Speed 55
// 转弯速度
#define Turn_Speed 85

// 角度转换到码盘常量↓

// 顺时针转换倍数
#define ClockWiseDigreeToEncoder 9.73f
// 逆时针转换倍数
#define CountClockWiseDigreeToEncoder 9.23f

// 循迹距离常量↓

// X轴中点循迹值
#define LongTrack_Value 1100
// Y轴中点循迹值
#define ShortTrack_Value 950 
// 到十字路口中心码盘值
#define ToCrossroadCenter 420 


// 循迹模式
typedef enum
{
    TrackMode_NONE = 0,
    TrackMode_NORMAL,
    TrackMode_ENCODER,
    TrackMode_BACK
} TrackMode_t;

// 根据码盘值运动的模式
typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_TurnByValue
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
// 定角度转向目标码盘值
extern uint16_t TurnByEncoder_Value;

extern uint8_t Stop_Flag;
extern int Car_Speed;
extern uint16_t temp_MP;


void Roadway_Check(void);
void Roadway_Flag_clean(void);
void Roadway_mp_syn(void);
void Roadway_nav_syn(void);
uint16_t Roadway_mp_Get(void);
void Control(int LSpeed, int RSpeed);
void Roadway_CheckTimInit(uint16_t arr, uint16_t psc);



void Get_Track(void);
void Get_DirectionWights(void);
void TRACK_LINE(void);
void Set_TunningDigree(uint16_t digree);



#endif
