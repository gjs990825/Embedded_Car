#if !defined(_A_STAR_H_)
#define _A_STAR_H_

#include "sys.h"

typedef struct RouteNode
{
	int8_t x;
	int8_t y;
	int8_t dir;
} RouteNode;

typedef struct Route_Task_Struct
{
	RouteNode node;
	void (*Task)(void);
} Route_Task_t;

typedef enum
{
	DIR_NOTSET = 0,
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
	DIR_RIGHT
} Driection_t;

typedef struct AStarNode
{
	int s_x;                    // x����(�������·����Ҫ)
	int s_y;                    // y����
	int s_g;                    // ��㵽�˵�ľ���( ��g��h���Եõ�f���˴�fʡ�ԣ�f=g+h )
	int s_h;                    // ��������Ԥ��Ĵ˵㵽�յ�ľ���
	int s_style;                // ������ͣ���ʼ�㣬�յ㣬�ϰ���
	struct AStarNode *s_parent; // ���ڵ�
	int s_is_in_closetable;     // �Ƿ���close����
	int s_is_in_opentable;      // �Ƿ���open����
} AStarNode, *pAStarNode;


extern RouteNode CurrentStaus;
extern Route_Task_t Route_Task[];
extern Route_Task_t Final_Route[];
extern int16_t Final_StepCount;
extern int8_t RouteTask_Finished[];
extern const uint8_t Route_TaskCount;

bool A_Star_GetRouteBewteenTasks(RouteNode current, Route_Task_t nextTask);


bool A_Star_GetRoute(void);


#endif // _A_STAR_H_
