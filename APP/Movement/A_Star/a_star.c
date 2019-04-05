#include "a_star.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "route.h"
#include "debug.h"
#include "malloc.h"

// ʹ��·�����
#define _A_STAR_ENABLE_OUTPUT_ 1

#define X_LENTH 7
#define Y_LENTH 7

typedef struct AStarNode
{
	int s_x;					// x����(�������·����Ҫ)
	int s_y;					// y����
	int s_g;					// ��㵽�˵�ľ���( ��g��h���Եõ�f���˴�fʡ�ԣ�f=g+h )
	int s_h;					// ��������Ԥ��Ĵ˵㵽�յ�ľ���
	int s_style;				// ������ͣ���ʼ�㣬�յ㣬�ϰ���
	struct AStarNode *s_parent; // ���ڵ�
	int s_is_in_closetable;		// �Ƿ���close����
	int s_is_in_opentable;		// �Ƿ���open����
} AStarNode, *pAStarNode;

// ����·��
int8_t path_array[X_LENTH * Y_LENTH][2];
// // ����
// int8_t step_count = -1;

// ���������
#define STARTNODE 1 // ���
#define ENDNODE 2   // �յ�
#define BARRIER 3   // �ϰ�

// ��ͼ����
const int8_t maze[X_LENTH][Y_LENTH] = {
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3},
	{0, 0, 0, 0, 0, 0, 0},
	{3, 0, 3, 0, 3, 0, 3}};

AStarNode map_maze[X_LENTH][Y_LENTH];	  // �������
pAStarNode open_table[X_LENTH * Y_LENTH];  // open��
pAStarNode close_table[X_LENTH * Y_LENTH]; // close��
int8_t open_node_count;					   // open���нڵ�����
int8_t close_node_count;				   // close���н������
AStarNode *start_node = NULL;			   // ��ʼ��
AStarNode *end_node = NULL;				   // ������

// ��������Ԫ��
void swap(int idx1, int idx2)
{
	pAStarNode tmp = open_table[idx1];
	open_table[idx1] = open_table[idx2];
	open_table[idx2] = tmp;
}

// �ѵ���
void adjust_heap(int nIndex)
{
	int curr = nIndex;
	int child = curr * 2 + 1;	// �õ�����idx( �±��0��ʼ��������������curr*2+1 )
	int parent = (curr - 1) / 2; // �õ�˫��idx

	if (nIndex < 0 || nIndex >= open_node_count)
	{
		return;
	}

	// ���µ���( Ҫ�Ƚ����Һ��Ӻ�cuur parent )
	while (child < open_node_count)
	{
		// С������˫��ֵС�ں���ֵ
		if (child + 1 < open_node_count && open_table[child]->s_g + open_table[child]->s_h > open_table[child + 1]->s_g + open_table[child + 1]->s_h)
		{
			++child; // �ж����Һ��Ӵ�С
		}

		if (open_table[curr]->s_g + open_table[curr]->s_h <= open_table[child]->s_g + open_table[child]->s_h)
		{
			break;
		}
		else
		{
			swap(child, curr);	// �����ڵ�
			curr = child;		  // ���жϵ�ǰ���ӽڵ�
			child = curr * 2 + 1; // ���ж�����
		}
	}

	if (curr != nIndex)
	{
		return;
	}

	// ���ϵ���( ֻ��Ҫ�Ƚ�cuur child��parent )
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

// �ж��ھӵ��Ƿ���Խ���open��
void insert_to_opentable(int x, int y, pAStarNode curr_node, pAStarNode end_node, int w)
{
	int i;

	if (map_maze[x][y].s_style != BARRIER) // �����ϰ���
	{
		if (!map_maze[x][y].s_is_in_closetable) // ���ڱձ���
		{
			if (map_maze[x][y].s_is_in_opentable) // ��open����
			{
				// ��Ҫ�ж��Ƿ���һ�����Ż���·��
				if (map_maze[x][y].s_g > curr_node->s_g + w) // ������Ż�
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

					adjust_heap(i); // ���������
				}
			}
			else // ����open��
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

// �����ھ�
// ����������4���ھӽ��в���
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

// ��ʼ����ͼ
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

// ���¼���·��ʱ�����
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

// ����·�� ���أ�1���ҵ�·�� 0��ʧ��
bool A_Star_CalaculateRoute(void)
{
	// ����ʹ��A*�㷨�õ�·��
	open_table[open_node_count++] = start_node; // ��ʼ�����open��

	start_node->s_is_in_opentable = 1; // ����open��
	start_node->s_g = 0;
	start_node->s_h = abs(end_node->s_x - start_node->s_x) + abs(end_node->s_y - start_node->s_y);
	start_node->s_parent = NULL;

	if (start_node->s_x == end_node->s_x && start_node->s_y == end_node->s_y)
	{
		return false;
	}

	bool is_found = false;
	AStarNode *curr_node; // ��ǰ��

	while (1)
	{
		curr_node = open_table[0];					   // open��ĵ�һ����һ����fֵ��С�ĵ�(ͨ��������õ���)
		open_table[0] = open_table[--open_node_count]; // ���һ����ŵ���һ���㣬Ȼ����жѵ���
		adjust_heap(0);								   // ������

		close_table[close_node_count++] = curr_node; // ��ǰ�����close��
		curr_node->s_is_in_closetable = 1;			 // �Ѿ���close������

		if (curr_node->s_x == end_node->s_x && curr_node->s_y == end_node->s_y) // �յ���close�У�����
		{
			is_found = true;
			break;
		}

		get_neighbors(curr_node, end_node); // ���ھӵĴ���

		if (open_node_count == 0) // û��·������
		{
			is_found = false;
			break;
		}
	}

	return is_found;
}

int A_Star_GetStepCount(void)
{
	AStarNode *curr_node; // ��ǰ��
	int top = -1;		  // ջ��

	curr_node = end_node;

	while (curr_node)
	{
		++top;
		path_array[top][0] = curr_node->s_x;
		path_array[top][1] = curr_node->s_y;
		curr_node = curr_node->s_parent;
	}

	return top;
}

// ����������·��
bool A_Star_GetTestRoute(RouteNode_t current, RouteNode_t next, RouteNode_t *finalRoute, uint8_t *routeCount)
{
	A_Star_SetStartEnd(current.x, current.y, next.x, next.y);
	if (A_Star_CalaculateRoute() == false)
	{
		return false;
	}
	int stepCount = A_Star_GetStepCount();

	*routeCount = 0;

	while (stepCount >= 0) // ȡ��·�������е����ݵ�finalRoute�У�����
	{

#if _A_STAR_ENABLE_OUTPUT_

		print_info((stepCount > 0) ? "(%d,%d)->" : "(%d,%d)", path_array[stepCount][0], path_array[stepCount][1]);
		delay_ms(50);

#endif // _A_STAR_ENABLE_OUTPUT_

		finalRoute[*routeCount].x = path_array[stepCount][0];
		finalRoute[*routeCount].y = path_array[stepCount][1];
		stepCount--;
		(*routeCount)++;
	}

#if _A_STAR_ENABLE_OUTPUT_

	print_info("\r\n");

#endif // _A_STAR_ENABLE_OUTPUT_

	return true;
}
