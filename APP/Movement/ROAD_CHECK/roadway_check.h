#ifndef __ROADWAY_H
#define __ROADWAY_H

#include "sys.h"

// �ٶ��趨������

// ѭ���ٶ�
#define Track_Speed 50 // 55
// ת���ٶ�
#define Turn_Speed 85 // 85
// ����ִ���е����ٶ�
#define Mission_Speed 30 // ���ٷ�ֹ��

// �Ƕ�ת�������̳�����

// ˳ʱ��ת������
#define ClockWiseDigreeToEncoder 10.54f //������ 10.1f
// ��ʱ��ת������
#define CountClockWiseDigreeToEncoder 9.25f // �� 9.83f

// ѭ�����볣����

// ÿ���׵�����ֵ
#define Centimeter_Value 28.94f
// X���е�ѭ��ֵ
static const int LongTrack_Value = (37.5f * Centimeter_Value);
// Y���е�ѭ��ֵ
static const int ShortTrack_Value = (30.0f * Centimeter_Value);
// ��ʮ��·����������ֵ
static const int ToCrossroadCenter = (15.2f * Centimeter_Value);

// ѭ��ģʽ
typedef enum
{
    TrackMode_NONE = 0,
    TrackMode_NORMAL,
    TrackMode_ENCODER,
    TrackMode_BACK,
    TrackMode_Turn
} TrackMode_t;

// ��������ֵ�˶���ģʽ
typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_TurnByValue
} Moving_ByEncoder_t;

// ѭ������Ϣ
extern int8_t Q7[7], H8[8];
// ѭ��������ĸ���
extern uint8_t NumberOfWhite;
// ����Ȩ��
extern int DirectionWights;
// ѭ��ģʽ
extern uint8_t Track_Mode;
// ��ֵǰ���ת��
extern Moving_ByEncoder_t Moving_ByEncoder;
// ���Ƕ�ת��Ŀ������ֵ
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
