#include "route.h"
#include "task.h"
#include "string.h"
#include "malloc.h"
#include "debug.h"
#include "my_lib.h"
#include "a_star.h"

// 当前位置状态
RouteNode_t CurrentStaus;
// 下一个位置和状态
RouteNode_t NextStatus;

// 任务和路径设定
RouteSetting_t Route_Task[] = {
    {.coordinate = "F7", .Task = Task_F7, .node.dir = DIR_UP},
    {.coordinate = "F6", .Task = Task_F6},
    {.coordinate = "D6", .Task = Task_D6},
    {.coordinate = "B6", .Task = Task_B6},
    {.coordinate = "B4", .Task = Task_B4},
    {.coordinate = "D4", .Task = Task_D4},
    {.coordinate = "F4", .Task = Task_F4},
    {.coordinate = "F2", .Task = Task_F2},
    {.coordinate = "D2", .Task = Task_D2},
    {.coordinate = "B2", .Task = Task_B2},
    // {.coordinate = "A2", .Task = Task_A2},
};

// 任务点个数
uint8_t ROUTE_TASK_NUMBER = GET_ARRAY_LENGEH(Route_Task);

// 模板，初始坐标如果这里不设定需要在自动运行前初始化
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

//////////////////////
// 坐标数据处理函数↓ //
//////////////////////

// 转换字符串到坐标点
RouteNode_t Coordinate_Covent(uint8_t coordinate[3])
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
    static const char *badCoordinate = "\0\0\0";
    static uint8_t tempCoordinate[3];

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
int8_t Get_TaskNumber(uint8_t coordinate[3], uint8_t *route, uint8_t nTimes)
{
    uint8_t count = 0;

    char *location = NULL;
    char *tempRoute = (char *)route;

    for (;;)
    {
        location = strstr(tempRoute, (char *)coordinate);
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

// 处理含有路径信息的字符串，去除无效信息
bool RouteString_Process(uint8_t *prefix, uint8_t *route, uint8_t *buffer)
{
    if (route == NULL)
        return false;

    uint16_t routeLen = strlen((char *)route);
    uint16_t prefixLen = 0;

    if (prefix != NULL)
        prefixLen = strlen((char *)prefix);

    uint8_t *tempbuffer = (uint8_t *)malloc(sizeof(uint8_t) * (routeLen + prefixLen));
    if (tempbuffer == NULL)
        return false;

    // 拷贝前缀和原始信息
    memcpy(tempbuffer, prefix, prefixLen);
    memcpy(tempbuffer + prefixLen, route, routeLen);

    uint8_t *pstr = buffer;

    // 查找符合条件的字符串并拷贝到buffer中
    for (uint16_t i = 0; i < (routeLen + prefixLen); i++)
    {
        if ((tempbuffer[i] >= 'A') && (tempbuffer[i] <= 'G'))
        {
            if ((tempbuffer[i + 1] >= '1') && (tempbuffer[i + 1] <= '7'))
            {
                pstr[0] = tempbuffer[i];
                pstr[1] = tempbuffer[i + 1];
                pstr += 2;

                // 匹配则跳过下一个字符
                i++;
            }
        }
    }
    pstr[0] = '\0';

    free(tempbuffer);

    return true;
}

// 判断坐标是否在路径
// 若则返回步数，否则返回-1
int8_t Is_ContainCoordinate(uint8_t *stringRoute, uint8_t coord[3])
{
    uint8_t length = strlen((char *)stringRoute) / 2;

    RouteNode_t *route = malloc(sizeof(RouteNode_t) * length);
    if (route == NULL)
        return -1;

    for (uint8_t i = 0; i < length; i++)
    {
        route[i] = Coordinate_Covent(&stringRoute[i * 2]);
    }

    RouteNode_t *tempRoute = malloc(sizeof(RouteNode_t) * 12); // 两点间最多12途径点
    if (tempRoute == NULL)
    {
        free(route);
        return -1;
    }

    uint8_t routeCount;
    uint8_t allRouteCount = 0;
    RouteNode_t coordinate = Coordinate_Covent(coord);

    for (uint8_t i = 0; i < length - 1; i++)
    {
        A_Star_GetRouteBetweenNodes(route[i], route[i + 1], tempRoute, &routeCount);

        for (uint8_t j = 0; j < routeCount; j++)
        {
            if ((tempRoute[j].x == coordinate.x) && (tempRoute[j].y == coordinate.y))
            {
                free(tempRoute);
                free(route);
                return allRouteCount + j;
            }
        }
        // 记录路径个数，去掉一个起始点
        allRouteCount += (routeCount - 1);
    }

    free(tempRoute);
    free(route);

    return -1;
}