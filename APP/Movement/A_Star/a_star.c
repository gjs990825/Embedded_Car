#include "a_star.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "route.h"
#include "debug.h"
#include "malloc.h"

// 使能路径输出
#define _A_STAR_ENABLE_OUTPUT_ 1

#define X_LENTH 7
#define Y_LENTH 7


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


// public :

// // 每个任务最多10个点 * 10
Route_Task_t Final_Route[ROUTE_TASK_NUMBER * 10];
// 路径计数
int16_t Final_StepCount = 0;


// prvate :

// 生成路径
int8_t path_array[X_LENTH * Y_LENTH][2];
// 步数
int8_t step_count = -1;


// 定义点类型
#define STARTNODE 1 // 起点
#define ENDNODE 2   // 终点
#define BARRIER 3   // 障碍

// 地图设置
const int8_t maze[X_LENTH][Y_LENTH] = {
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3}};

AStarNode map_maze[X_LENTH][Y_LENTH];	   // 结点数组
pAStarNode open_table[X_LENTH * Y_LENTH];  // open表
pAStarNode close_table[X_LENTH * Y_LENTH]; // close表
int8_t open_node_count;					   // open表中节点数量
int8_t close_node_count;				   // close表中结点数量
AStarNode *start_node = NULL;			   // 起始点
AStarNode *end_node = NULL;				   // 结束点

// 交换两个元素
void swap(int idx1, int idx2)
{
	pAStarNode tmp = open_table[idx1];
	open_table[idx1] = open_table[idx2];
	open_table[idx2] = tmp;
}

// 堆调整
void adjust_heap(int nIndex)
{
	int curr = nIndex;
	int child = curr * 2 + 1;	// 得到左孩子idx( 下标从0开始，所有做孩子是curr*2+1 )
	int parent = (curr - 1) / 2; // 得到双亲idx

	if (nIndex < 0 || nIndex >= open_node_count)
	{
		return;
	}

	// 往下调整( 要比较左右孩子和cuur parent )
	while (child < open_node_count)
	{
		// 小根堆是双亲值小于孩子值
		if (child + 1 < open_node_count && open_table[child]->s_g + open_table[child]->s_h > open_table[child + 1]->s_g + open_table[child + 1]->s_h)
		{
			++child; // 判断左右孩子大小
		}

		if (open_table[curr]->s_g + open_table[curr]->s_h <= open_table[child]->s_g + open_table[child]->s_h)
		{
			break;
		}
		else
		{
			swap(child, curr);	// 交换节点
			curr = child;		  // 再判断当前孩子节点
			child = curr * 2 + 1; // 再判断左孩子
		}
	}

	if (curr != nIndex)
	{
		return;
	}

	// 往上调整( 只需要比较cuur child和parent )
	while (curr != 0)
	{
		if (open_table[curr]->s_g + open_table[curr]->s_h >= open_table[parent]->s_g + open_table[parent]->s_h)
		{
			break;
		}
		else
		{
			swap(curr, parent);
			curr = parent;
			parent = (curr - 1) / 2;
		}
	}
}

// 判断邻居点是否可以进入open表
void insert_to_opentable(int x, int y, pAStarNode curr_node, pAStarNode end_node, int w)
{
	int i;

	if (map_maze[x][y].s_style != BARRIER) // 不是障碍物
	{
		if (!map_maze[x][y].s_is_in_closetable) // 不在闭表中
		{
			if (map_maze[x][y].s_is_in_opentable) // 在open表中
			{
				// 需要判断是否是一条更优化的路径
				if (map_maze[x][y].s_g > curr_node->s_g + w) // 如果更优化
				{
					map_maze[x][y].s_g = curr_node->s_g + w;
					map_maze[x][y].s_parent = curr_node;

					for (i = 0; i < open_node_count; ++i)
					{
						if (open_table[i]->s_x == map_maze[x][y].s_x && open_table[i]->s_y == map_maze[x][y].s_y)
						{
							break;
						}
					}

					adjust_heap(i); // 下面调整点
				}
			}
			else // 不在open中
			{
				map_maze[x][y].s_g = curr_node->s_g + w;
				map_maze[x][y].s_h = abs(end_node->s_x - x) + abs(end_node->s_y - y);
				map_maze[x][y].s_parent = curr_node;
				map_maze[x][y].s_is_in_opentable = 1;
				open_table[open_node_count++] = &(map_maze[x][y]);
			}
		}
	}
}

// 查找邻居
// 对上下左右4个邻居进行查找
void get_neighbors(pAStarNode curr_node, pAStarNode end_node)
{
	int x = curr_node->s_x;
	int y = curr_node->s_y;

	if ((x + 1) >= 0 && (x + 1) < X_LENTH && y >= 0 && y < X_LENTH)
	{
		insert_to_opentable(x + 1, y, curr_node, end_node, 1);
	}

	if ((x - 1) >= 0 && (x - 1) < X_LENTH && y >= 0 && y < X_LENTH)
	{
		insert_to_opentable(x - 1, y, curr_node, end_node, 1);
	}

	if (x >= 0 && x < X_LENTH && (y + 1) >= 0 && (y + 1) < X_LENTH)
	{
		insert_to_opentable(x, y + 1, curr_node, end_node, 1);
	}

	if (x >= 0 && x < X_LENTH && (y - 1) >= 0 && (y - 1) < X_LENTH)
	{
		insert_to_opentable(x, y - 1, curr_node, end_node, 1);
	}
}

// 初始化地图
void A_Star_InitMap(void)
{
	for (int8_t i = 0; i < X_LENTH; ++i)
	{
		for (int8_t j = 0; j < Y_LENTH; ++j)
		{
			map_maze[i][j].s_g = 0;
			map_maze[i][j].s_h = 0;
			map_maze[i][j].s_is_in_closetable = 0;
			map_maze[i][j].s_is_in_opentable = 0;
			map_maze[i][j].s_style = maze[i][j];
			map_maze[i][j].s_x = i;
			map_maze[i][j].s_y = j;
			map_maze[i][j].s_parent = NULL;
		}
	}
}

// 重新计算路径时候调用
void A_Star_SetStartEnd(int start_x, int start_y, int end_x, int end_y)
{
	A_Star_InitMap();

	// memset(open_table, 0, sizeof(open_table));
	// memset(close_table, 0, sizeof(close_table));

	open_node_count = 0;
	close_node_count = 0;

	map_maze[start_x][start_y].s_style = STARTNODE;
	map_maze[end_x][end_y].s_style = ENDNODE;

	start_node = &(map_maze[start_x][start_y]);
	end_node = &(map_maze[end_x][end_y]);
}

// 计算路径 返回：1：找到路径 0：失败
bool A_Star_CalaculateRoute(void)
{
	// 下面使用A*算法得到路径
	open_table[open_node_count++] = start_node; // 起始点加入open表

	start_node->s_is_in_opentable = 1; // 加入open表
	start_node->s_g = 0;
	start_node->s_h = abs(end_node->s_x - start_node->s_x) + abs(end_node->s_y - start_node->s_y);
	start_node->s_parent = NULL;

	if (start_node->s_x == end_node->s_x && start_node->s_y == end_node->s_y)
	{
		return false;
	}

	bool is_found = false;
	AStarNode *curr_node; // 当前点

	while (1)
	{
		curr_node = open_table[0];					   // open表的第一个点一定是f值最小的点(通过堆排序得到的)
		open_table[0] = open_table[--open_node_count]; // 最后一个点放到第一个点，然后进行堆调整
		adjust_heap(0);								   // 调整堆

		close_table[close_node_count++] = curr_node; // 当前点加入close表
		curr_node->s_is_in_closetable = 1;			 // 已经在close表中了

		if (curr_node->s_x == end_node->s_x && curr_node->s_y == end_node->s_y) // 终点在close中，结束
		{
			is_found = true;
			break;
		}

		get_neighbors(curr_node, end_node); // 对邻居的处理

		if (open_node_count == 0) // 没有路径到达
		{
			is_found = false;
			break;
		}
	}

	return is_found;
}

void A_Star_GetStepCount(void)
{
	AStarNode *curr_node; // 当前点
	int top = -1;		  // 栈顶

	curr_node = end_node;

	while (curr_node)
	{
		++top;
		path_array[top][0] = curr_node->s_x;
		path_array[top][1] = curr_node->s_y;
		curr_node = curr_node->s_parent;
	}

	step_count = top;
}

#if _A_STAR_ENABLE_OUTPUT_

void A_Star_PrintRoute(void)
{
	int top = step_count;

	if (step_count == -1)
	{
		print_info("NO route or didn't caculate\r\n");
		return;
	}

	while (top >= 0) // 输出路径
	{
		if (top > 0)
		{
			print_info("(%d,%d)->", path_array[top][0], path_array[top][1]);
		}
		else
		{
			print_info("(%d,%d)", path_array[top][0], path_array[top][1]);
		}
		top--;
		delay_ms(50); // CAN总线响应时间
	}
	print_info("\r\n");
}

#endif // _A_STAR_ENABLE_OUTPUT_

void Pop_Array(void)
{
	int i = step_count;

	while (i > 0)
	{
		Final_Route[Final_StepCount].node.x = path_array[i][0];
		Final_Route[Final_StepCount].node.y = path_array[i][1];

		i--;
		Final_StepCount++;
	}
}

// // 生成两个任务间的路线 返回 0 错误 1 成功
// bool A_Star_GetRouteBewteenTasks(RouteNode_t current, Route_Task_t nextTask)
// {
// 	Final_StepCount = 0; // 清空上一次路径计算数据
// 	step_count = 0;

// 	A_Star_SetStartEnd(current.x, current.y, nextTask.node.x, nextTask.node.y);
// 	if (A_Star_CalaculateRoute() == false)
// 	{
// 		return false;
// 	}
// 	A_Star_GetStepCount();
// 	A_Star_PrintRoute();

// 	int count = step_count;
// 	while (count >= 0)
// 	{
// 		Final_Route[Final_StepCount].Task = NULL;
// 		Final_Route[Final_StepCount].node.x = path_array[count][0]; // 添加点
// 		Final_Route[Final_StepCount].node.y = path_array[count][1];
// 		count--;
// 		Final_StepCount++;
// 	}
// 	Final_Route[step_count].Task = nextTask.Task; // 添加任务

// 	return true;
// }

// // 生成全部路线路线 返回 0 错误 1 成功
// bool A_Star_GetRoute(void)
// {
// 	uint16_t tmp = ROUTE_TASK_NUMBER;

// 	CurrentStaus = Route_Task[0].node; // 初始化当前位置

// 	for (uint16_t i = 0; i < tmp - 1; i++)
// 	{
// 		A_Star_SetStartEnd(Route_Task[i].node.x, Route_Task[i].node.y, Route_Task[i + 1].node.x, Route_Task[i + 1].node.y);
// 		if (A_Star_CalaculateRoute() == false)
// 		{
// 			return false;
// 		}
// 		A_Star_GetStepCount();
// 		A_Star_PrintRoute();

// 		int count = step_count;
// 		while (count > 0)
// 		{
// 			if (count == step_count) // 节点开始，有任务
// 			{
// 				Final_Route[Final_StepCount].Task = Route_Task[i].Task; // 添加任务
// 			}
// 			else
// 			{
// 				Final_Route[Final_StepCount].Task = NULL;
// 			}
// 			Final_Route[Final_StepCount].node.x = path_array[count][0]; // 添加点
// 			Final_Route[Final_StepCount].node.y = path_array[count][1];
// 			count--;
// 			Final_StepCount++;
// 		}
// 	}

// 	// 添加终点
// 	Final_Route[Final_StepCount] = Route_Task[tmp - 1]; // 添加终点
// 	Final_StepCount++;

// 	return true;
// }

// int main()
// {
// 	print_info("%d\r\n", A_Star_GetRoute());

// 	print_info("%d\r\n", Final_StepCount);

// 	for (size_t i = 0; i < Final_StepCount; i++)
// 	{
// 		print_info("(%d,%d)-->", Final_Route[i].x, Final_Route[i].y);
// 	}

// 	return 0;
// }


// 生成两个任务间的路线 返回 0 错误 1 成功
bool A_Star_GetTestRoute(RouteNode_t current, RouteNode_t next, RouteNode_t *finalRoute, uint8_t *routeCount)
{
	Final_StepCount = 0; // 清空上一次路径计算数据
	step_count = 0;

	A_Star_SetStartEnd(current.x, current.y, next.x, next.y);
	if (A_Star_CalaculateRoute() == false)
	{
		return false;
	}
	A_Star_GetStepCount();
	A_Star_PrintRoute();

	int count = step_count;
	while (count >= 0)
	{
		finalRoute[Final_StepCount].x = path_array[count][0]; // 添加点
		finalRoute[Final_StepCount].y = path_array[count][1];
		count--;
		Final_StepCount++;
	}

	*routeCount = Final_StepCount;

	return true;
}
