#if !defined(_A_STAR_H_)
#define _A_STAR_H_

#include "sys.h"

#define DIR_UP 1
#define DIR_LEFT 2
#define DIR_DOWN 3
#define DIR_RIGHT 4

typedef struct RouteNode
{
	int8_t x;
	int8_t y;
	int8_t dir;
} RouteNode;

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
extern RouteNode Final_Route[];
extern int Final_StepCount;

bool A_Star_GetRoute(void);

void Auto_Drive(void);

#endif // _A_STAR_H_
