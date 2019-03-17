#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"
#include "Timer.h"

// Stop_Flag��ö��
typedef enum
{
	TRACKING = 0x00,		// ѭ��״̬
	CROSSROAD = 0x01,		// ʮ��·��
	TURNCOMPLETE = 0x02,	// ת�����
	FORBACKCOMPLETE = 0x03, // ǰ���������(����ѭ��)
	OUTTRACK = 0x04			// ����
} StopFlag_t;

// �ȴ�ĳ����־λ��ע�⣺��ָ��û�еȴ���ʱ�������
#define WaitForFlag(flag, status) \
	do                            \
	{                             \
		while (flag != status)    \
		{                         \
		};                        \
	} while (0)

// �ȴ�ĳ����־λ�����趨��ʱʱ��
#define WaitForFlagInMs(flag, status, timeout)                                       \
	do                                                                               \
	{                                                                                \
		uint32_t startStamp = Get_GlobalTimeStamp();                                  \
		while ((Get_GlobalTimeStamp() < (startStamp + timeout)) && (flag != status)) \
		{                                                                            \
		};                                                                           \
	} while (0)

// void WaitForFlagInMs(uint8_t flag, uint8_t status, uint16_t timeout)
// {
// 	do
// 	{
// 		uint8_t startStamp = Get_GlobalTimeStamp();
// 		while ((Get_GlobalTimeStamp() < (startStamp + timeout)) && (flag != status))
// 		{
// 		};
// 	} while (0);
// }

// �ȴ�ִ����ɡ�һ������ת��ѭ���Ȳ��ᳬʱ������
#define ExcuteAndWait(action, Flag, waitStatus) \
	do                                          \
	{                                           \
		action;                                 \
		WaitForFlag(Flag, waitStatus);          \
		Stop();                                 \
	} while (0)

// �Զ�ִ��
void Auto_Run(void);
void Auto_RouteTask(RouteNode current, uint8_t taskN);
void Go_ToNextNode(Route_Task_t next);

// �����˶�����
void Stop(void);
void Go_Ahead(int speed, uint16_t mp);
void Back_Off(int speed, uint16_t mp);
void Turn_ByEncoder(int16_t digree);
void Track_ByEncoder(int speed, uint16_t setMP);
void Start_Tracking(int speed);

#endif // _MOVEMENT_H_
