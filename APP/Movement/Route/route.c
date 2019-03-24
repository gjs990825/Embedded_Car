#include "route.h"
#include "string.h"

// 主车当前位置状态
RouteNode CurrentStaus;
// 主车下一个位置和状态
RouteNode NextStatus;
// 主车任务完成情况
int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};


// 转换字符串到坐标点 (需要优化)
RouteNode Coordinate_Covent(uint8_t str[2])
{
    RouteNode outNode;
    outNode.dir = DIR_NOTSET;

    switch (str[0])
    {
    case 'A':
        outNode.x = 0;
        break;
    case 'B':
        outNode.x = 1;
        break;
    case 'C':
        outNode.x = 2;
        break;
    case 'D':
        outNode.x = 3;
        break;
    case 'E':
        outNode.x = 4;
        break;
    case 'F':
        outNode.x = 5;
        break;
    case 'G':
        outNode.x = 6;
        break;

    default:
        outNode.x = -1;
        break;
    }
    switch (str[1])
    {
    case '1':
        outNode.y = 6;
        break;
    case '2':
        outNode.y = 5;
        break;
    case '3':
        outNode.y = 4;
        break;
    case '4':
        outNode.y = 3;
        break;
    case '5':
        outNode.y = 2;
        break;
    case '6':
        outNode.y = 1;
        break;
    case '7':
        outNode.y = 0;
        break;
    default:
        outNode.y = -1;
        break;
    }

    return outNode;
}

// 转换坐标点到字符串
uint8_t *ReCoordinate_Covent(uint8_t x, uint8_t y)
{
    static uint8_t tempCoord[2];

    switch (x)
    {
    case 0:
        tempCoord[0] = 'A';
        break;
    case 1:
        tempCoord[0] = 'B';
        break;
    case 2:
        tempCoord[0] = 'C';
        break;
    case 3:
        tempCoord[0] = 'D';
        break;
    case 4:
        tempCoord[0] = 'E';
        break;
    case 5:
        tempCoord[0] = 'F';
        break;
    case 6:
        tempCoord[0] = 'G';
        break;
    default:
        tempCoord[0] = '\0';
        break;
    }

    switch (y)
    {
    case 0:
        tempCoord[1] = '7';
        break;
    case 1:
        tempCoord[1] = '6';
        break;
    case 2:
        tempCoord[1] = '5';
        break;
    case 3:
        tempCoord[1] = '4';
        break;
    case 4:
        tempCoord[1] = '3';
        break;
    case 5:
        tempCoord[1] = '2';
        break;
    case 6:
        tempCoord[1] = '1';
        break;

    default:
        tempCoord[1] = '\0';
        break;
    }

    return tempCoord;
}

// 获取任务点的编号，-1为错误
int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route)
{
	uint8_t str1[3];
	memcpy(str1, coordinate, 2);
	str1[2] = '\0';

	// 返回坐标指针与路径起始点的差值 / 2
	char *number = strstr((char *)route, (char *)str1);
	if (number == NULL)
	{
		return -1;
	}
	return (strstr((char *)route, (char *)str1) - (char *)route) / 2;
}
