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
	int s_x;                    // x坐标(最终输出路径需要)
	int s_y;                    // y坐标
	int s_g;                    // 起点到此点的距离( 由g和h可以得到f，此处f省略，f=g+h )
	int s_h;                    // 启发函数预测的此点到终点的距离
	int s_style;                // 结点类型：起始点，终点，障碍物
	struct AStarNode *s_parent; // 父节点
	int s_is_in_closetable;     // 是否在close表中
	int s_is_in_opentable;      // 是否在open表中
} AStarNode, *pAStarNode;


extern RouteNode CurrentStaus;
extern RouteNode Final_Route[];
extern int Final_StepCount;

bool A_Star_GetRoute(void);

void Auto_Drive(void);

#endif // _A_STAR_H_
