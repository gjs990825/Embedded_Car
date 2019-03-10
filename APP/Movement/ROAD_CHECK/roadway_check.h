#ifndef __ROADWAY_H
#define __ROADWAY_H
//#include "stm32f4xx.h"
#include "sys.h"

// �ٶȳ�����

// ѭ���ٶ�
#define Track_Speed 50 // 55
// ת���ٶ�
#define Turn_Speed 70 // 85

// �Ƕ�ת�������̳�����

// ˳ʱ��ת������
#define ClockWiseDigreeToEncoder 10.1f
// ��ʱ��ת������
#define CountClockWiseDigreeToEncoder 9.47f

// ѭ�����볣����

// ÿ��������ֵ
#define Centimeter_Value 30
// X���е�ѭ��ֵ
#define LongTrack_Value 1125 // 1100 (37.5 * 30)
// Y���е�ѭ��ֵ
#define ShortTrack_Value 900 // 950 (30 * 30)
// ��ʮ��·����������ֵ
#define ToCrossroadCenter 390 // 420 


// ѭ��ģʽ
typedef enum
{
    TrackMode_NONE = 0,
    TrackMode_NORMAL,
    TrackMode_ENCODER,
    TrackMode_BACK
} TrackMode_t;

// ��������ֵ�˶���ģʽ
typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_TurnByValue
} Moving_ByEncoder_t;

// ѭ����Ϣ
extern int8_t Q7[7], H8[8];
// ��ɫ����
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
