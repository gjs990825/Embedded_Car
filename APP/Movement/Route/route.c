#include "route.h"
#include "task.h"
#include "string.h"
#include "malloc.h"
#include "debug.h"
#include "my_lib.h"

// 当前位置状态
RouteNode_t CurrentStaus;
// 下一个位置和状态
RouteNode_t NextStatus;
// 任务完成情况
// int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};

// 坐标模板，初始坐标如果这里不设定需要在自动运行前初始化
RouteSetting_t Route_Task[] = {
    {.coordinate = "F7", .Task = NULL, .node.dir = DIR_UP},
    {.coordinate = "F6", .Task = NULL},
    {.coordinate = "D6", .Task = NULL},
    {.coordinate = "B6", .Task = NULL},
    {.coordinate = "B4", .Task = NULL},
    {.coordinate = "B2", .Task = NULL},
    {.coordinate = "D2", .Task = NULL},
    {.coordinate = "F2", .Task = NULL},
    {.coordinate = "F4", .Task = NULL},
    {.coordinate = "D4", .Task = NULL},
    {.coordinate = "D6", .Task = NULL},
    {.coordinate = "F6", .Task = NULL},
    {.coordinate = "F7", .Task = NULL},
};

// // 任务点设定
// RouteSetting_t Route_Task[] = {
//     {.coordinate = "F7", .Task = Start_Task, .node.dir = DIR_UP},
//     {.coordinate = "F6", .Task = Task_F6},
//     {.coordinate = "D6", .Task = Task_3_1},
//     {.coordinate = "B6", .Task = NULL},
//     {.coordinate = "B4", .Task = Task_1_3},
//     {.coordinate = "B2", .Task = Task_1_5},
//     {.coordinate = "D2", .Task = Task_3_5},
//     {.coordinate = "F2", .Task = Task_5_5},
//     {.coordinate = "F4", .Task = NULL},
//     {.coordinate = "D4", .Task = NULL},
//     {.coordinate = "D6", .Task = Task_3_1_2},
//     {.coordinate = "F6", .Task = Task_F6_2},
//     // {.coordinate = "F7", .Task = NULL}, // 入库点
// };
// 任务点个数
uint8_t ROUTE_TASK_NUMBER = GET_ARRAY_LENGEH(Route_Task);

// RFID 寻卡测试用路径
RouteSetting_t RFID_TestRoute[] = {
    {.coordinate = "B7", .Task = NULL, .node.dir = DIR_UP},
    {.coordinate = "B6", .Task = Task_RFIDTestStart},
    {.coordinate = "B5", .Task = NULL},
    {.coordinate = "B4", .Task = NULL},
    {.coordinate = "C4", .Task = Task_RFIDTestEnd},
    {.coordinate = "D4", .Task = NULL},
};
uint8_t RFID_TESTROUTE_NUMBER = GET_ARRAY_LENGEH(RFID_TestRoute);

// 转换字符串到坐标点
RouteNode_t Coordinate_Covent(uint8_t coordinate[2])
{
    static const RouteNode_t badNode = {.x = -1, .y = -1, .dir = DIR_NOTSET};
    RouteNode_t outNode;
    outNode.dir = DIR_NOTSET;

    if ((coordinate[0] >= 'A') && (coordinate[0] <= 'G'))
    {
        outNode.x = coordinate[0] - 'A';
    }
    else // 不合法的x坐标
        return badNode;

    if ((coordinate[1] >= '1') && (coordinate[1] <= '7'))
    {
        outNode.y = '7' - coordinate[1];
    }
    else // 不合法的y坐标
        return badNode;

    return outNode;
}

// 转换坐标点到字符串
uint8_t *ReCoordinate_Covent(int8_t x, int8_t y)
{
    static const char *badCoordinate = "\0\0";
    static uint8_t tempCoordinate[2];

    if (x >= 0 && x <= 6)
    {
        tempCoordinate[0] = 'A' + x;
    }
    else // 不合法的x坐标
        return (uint8_t *)badCoordinate;

    if (y >= 0 && y <= 6)
    {
        tempCoordinate[1] = '7' - y;
    }
    else // 不合法的y坐标
        return (uint8_t *)badCoordinate;

    return tempCoordinate;
}

// 获取任务点在路径中出现第n次的位置，返回-1为错误
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
            tempRoute = location + 2; // 指向下一个坐标点的字符
            if (++count >= nTimes)
                return (location - (char *)route) / 2; // 算出第n次出现的任务位置
        }
    }
}

// 从设定路径中的字符串生成坐标路径
bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count)
{
    RouteNode_t tempNode;

    for (uint8_t i = 0; i < count; i++)
    {
        tempNode = Coordinate_Covent(routeSetting[i].coordinate);
        routeSetting[i].node.x = tempNode.x; // 防止设定的方向被变更
        routeSetting[i].node.y = tempNode.y;

        if (routeSetting[i].node.x == -1 || routeSetting[i].node.y == -1)
            return false; // 无效节点，退出

        print_info((i < count - 1) ? "(%d, %d)->" : "(%d, %d)", routeSetting[i].node.x, routeSetting[i].node.y);
        delay_ms(50);
    }

    print_info("\r\n");

    return true;
}
