#include "route.h"
#include "string.h"
#include "malloc.h"
#include "debug.h"
#include "my_lib.h"

// ������ǰλ��״̬
RouteNode_t CurrentStaus;
// ������һ��λ�ú�״̬
RouteNode_t NextStatus;
// // ��������������
// int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};

// ������趨
RouteSetting_t Route_Task[] = {
    {.coordinate = "F7", .Task = Start_Task, .node.dir = DIR_UP},
    {.coordinate = "F6", .Task = Task_F6},
    {.coordinate = "D6", .Task = Task_3_1},
    {.coordinate = "B6", .Task = NULL},
    {.coordinate = "B4", .Task = Task_1_3},
    {.coordinate = "B2", .Task = Task_1_5},
    {.coordinate = "D2", .Task = Task_3_5},
    {.coordinate = "F2", .Task = Task_5_5},
    {.coordinate = "F4", .Task = NULL},
    {.coordinate = "D4", .Task = NULL},
    {.coordinate = "D6", .Task = Task_3_1_2},
    {.coordinate = "F6", .Task = Task_F6_2},
    // {.coordinate = "F7", .Task = NULL},
};
// ��������
uint8_t ROUTE_TASK_NUMBER = GET_ARRAY_LENGEH(Route_Task);

// // ����ģ�壬��ʼ����������ﲻ�趨��Ҫ���Զ�����ǰ��ʼ��
// RouteSetting_t Route_Task[] = {
//     {.coordinate = "F7", .Task = NULL, .node.dir = DIR_UP},
//     {.coordinate = "F6", .Task = NULL},
//     {.coordinate = "D6", .Task = NULL},
//     {.coordinate = "B6", .Task = NULL},
//     {.coordinate = "B4", .Task = NULL},
//     {.coordinate = "B2", .Task = NULL},
//     {.coordinate = "D2", .Task = NULL},
//     {.coordinate = "F2", .Task = NULL},
//     {.coordinate = "F4", .Task = NULL},
//     {.coordinate = "D4", .Task = NULL},
//     {.coordinate = "D6", .Task = NULL},
//     {.coordinate = "F6", .Task = NULL},
//     {.coordinate = "F7", .Task = NULL},
// };

// RFID Ѱ��������·��
RouteSetting_t RFID_TestRoute[] = {
    {.coordinate = "B7", .Task = NULL, .node.dir = DIR_UP},
    {.coordinate = "B6", .Task = Task_RFID_RoadSectionTrue},
    {.coordinate = "B5", .Task = NULL},
    {.coordinate = "B4", .Task = NULL},
    {.coordinate = "C4", .Task = Task_RFID_RoadSectionFalse},
    {.coordinate = "D4", .Task = NULL},
};
uint8_t RFID_TESTROUTE_NUMBER = GET_ARRAY_LENGEH(RFID_TestRoute);

// ת���ַ���������� (��Ҫ�Ż�)
RouteNode_t Coordinate_Covent(uint8_t str[2])
{
    RouteNode_t outNode;
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

// ת������㵽�ַ���
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

// ��ȡ�����ı�ţ�-1Ϊ����
int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route)
{
    uint8_t str1[3];
    memcpy(str1, coordinate, 2);
    str1[2] = '\0';

    // ��������ָ����·����ʼ��Ĳ�ֵ / 2
    char *number = strstr((char *)route, (char *)str1);
    if (number == NULL)
    {
        return -1;
    }
    return (strstr((char *)route, (char *)str1) - (char *)route) / 2;
}

// int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route, uint8_t nTimes)
// {
// 	char str1[3];
// 	uint8_t count = 0;
// 	memcpy(str1, coordinate, 2);
// 	str1[2] = '\0';

	
// 	char *location = NULL;
// 	char *str = (char *)route;

// 	for (;;)
// 	{
// 		location = strstr(str, str1);
// 		if (location == NULL)
// 			break;
// 		else
// 		{
// 			str = location + 2; // ָ����һ���������ַ�
// 			if (++count >= nTimes)
// 				break;
// 		}
// 	}

// 	if (count >= nTimes)
// 	{
// 		// ��������ָ����·����ʼ��Ĳ�ֵ / 2
// 		return (location - (char *)route) / 2;
// 	}
// 	else
// 		return -1;
// }


// ���趨·���е��ַ�����������·��
bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count)
{
    RouteNode_t tempNode;

    for (uint8_t i = 0; i < count; i++)
    {
        tempNode = Coordinate_Covent(routeSetting[i].coordinate);
        routeSetting[i].node.x = tempNode.x; // ��ֹ�趨�ķ��򱻱��
        routeSetting[i].node.y = tempNode.y;

        if (routeSetting[i].node.x == -1 || routeSetting[i].node.y == -1)
            return false; // ��Ч�ڵ㣬�˳�

        print_info("(%d, %d)->", i, routeSetting[i].node.x, routeSetting[i].node.y);
        delay_ms(50);
    }

    print_info("\r\n");

    return true;
}
