#ifndef __ROADWAY_H
#define __ROADWAY_H

#include "sys.h"

// 速度设定常量↓

// 循迹速度
#define Track_Speed 50 // 55
// 转弯速度
#define Turn_Speed 85 // 85
// 任务执行中调整速度
#define Mission_Speed 30 // 慢速防止打滑

// 角度转换到码盘常量↓

// 顺时针转换倍数
#define ClockWiseDigreeToEncoder 10.54f //旧赛道 10.1f
// 逆时针转换倍数
#define CountClockWiseDigreeToEncoder 9.25f // 旧 9.83f

// 循迹距离常量↓

// 每厘米的码盘值
#define Centimeter_Value 28.94f
// X轴中点循迹值
static const int LongTrack_Value = (37.5f * Centimeter_Value);
// Y轴中点循迹值
static const int ShortTrack_Value = (30.0f * Centimeter_Value);
// 到十字路口中心码盘值
static const int ToCrossroadCenter = (15.2f * Centimeter_Value);

// 循迹模式
typedef enum
{
    TrackMode_NONE = 0,
    TrackMode_NORMAL,
    TrackMode_ENCODER,
    TrackMode_BACK,
    TrackMode_Turn
} TrackMode_t;

// 根据码盘值运动的模式
typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_TurnByValue
} Moving_ByEncoder_t;

// 循迹灯信息
extern int8_t Q7[7], H8[8];
// 循迹灯亮起的个数
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
