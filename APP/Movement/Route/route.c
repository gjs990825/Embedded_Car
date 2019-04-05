#include "route.h"
#include "string.h"
#include "malloc.h"
#include "debug.h"
#include "my_lib.h"

// ��ǰλ��״̬
RouteNode_t CurrentStaus;
// ��һ��λ�ú�״̬
RouteNode_t NextStatus;
// ����������
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
    // {.coordinate = "F7", .Task = NULL}, // ����
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

// ת���ַ����������
RouteNode_t Coordinate_Covent(uint8_t coordinate[2])
{
    static const RouteNode_t badNode = {.x = -1, .y = -1, .dir = DIR_NOTSET};
    RouteNode_t outNode;
    outNode.dir = DIR_NOTSET;

    if ((coordinate[0] >= 'A') && (coordinate[0] <= 'G'))
    {
        outNode.x = coordinate[0] - 'A';
    }
    else // ���Ϸ���x����
        return badNode;

    if ((coordinate[1] >= '1') && (coordinate[1] <= '7'))
    {
        outNode.y = '7' - coordinate[1];
    }
    else // ���Ϸ���y����
        return badNode;

    return outNode;
}

// ת������㵽�ַ���
uint8_t *ReCoordinate_Covent(int8_t x, int8_t y)
{
    static const char *badCoordinate = "\0\0";
    static uint8_t tempCoordinate[2];

    if (x >= 0 && x <= 6)
    {
        tempCoordinate[0] = 'A' + x;
    }
    else // ���Ϸ���x����
        return (uint8_t *)badCoordinate;

    if (y >= 0 && y <= 6)
    {
        tempCoordinate[1] = '7' - y;
    }
    else // ���Ϸ���y����
        return (uint8_t *)badCoordinate;

    return tempCoordinate;
}

// ��ȡ�������·���г��ֵ�n�ε�λ�ã�����-1Ϊ����
int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route, uint8_t nTimes)
{
    uint8_t count = 0;
    char tempCoordinate[3];

    memcpy(tempCoordinate, coordinate, 2);
    tempCoordinate[2] = '\0';

    char *location = NULL;
    char *tempRoute = (char *)route;

    for (;;)
    {
        location = strstr(tempRoute, tempCoordinate);
        if (location == NULL)
            return -1;
        else
        {
            tempRoute = location + 2; // ָ����һ���������ַ�
            if (++count >= nTimes)
                return (location - (char *)route) / 2; // �����n�γ��ֵ�����λ��
        }
    }
}

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

        print_info((i < count - 1) ? "(%d, %d)->" : "(%d, %d)", routeSetting[i].node.x, routeSetting[i].node.y);
        delay_ms(50);
    }

    print_info("\r\n");

    return true;
}
