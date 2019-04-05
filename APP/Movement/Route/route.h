#if !defined(_ROUTE_H_)
#define _ROUTE_H_

#include "sys.h"
#include "task.h"
#include "stddef.h"

// ·����ṹ
typedef struct RouteNode_Struct
{
	int8_t x;
	int8_t y;
	int8_t dir; // �õ����࣬��Ҫ����
} RouteNode_t;

// ·���㣨����������Ϣ��
typedef struct Route_Task_Struct
{
	RouteNode_t node;
	void (*Task)(void);
} Route_Task_t;

// ���򣨳�ͷ����
typedef enum
{
	DIR_NOTSET = 0,
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
	DIR_RIGHT
} Driection_t;

// �趨����㣨ת���ַ�������������Ϣ��
typedef struct RouteSetting_Struct
{
	uint8_t coordinate[2];
	RouteNode_t node;
	void (*Task)(void);
} RouteSetting_t;

// �����趨���������
extern RouteSetting_t Route_Task[];
extern uint8_t ROUTE_TASK_NUMBER;

// Ѱ��������·��
extern RouteSetting_t RFID_TestRoute[];
extern uint8_t RFID_TESTROUTE_NUMBER;

extern RouteNode_t CurrentStaus;
extern RouteNode_t NextStatus;

// // ����������
// extern int8_t RouteTask_Finished[ROUTE_TASK_NUMBER];

RouteNode_t Coordinate_Covent(uint8_t str[2]);
uint8_t *ReCoordinate_Covent(int8_t x, int8_t y);
bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count);
int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route, uint8_t nTimes);

#endif // _ROUTE_H_
