#if !defined(_ROUTE_H_)
#define _ROUTE_H_

#include "sys.h"
#include "a_star.h"
#include "task.h"
#include "stddef.h"

// 任务个数
#define ROUTE_TASK_NUMBER (sizeof(Route_Task) / sizeof(Route_Task[0]))

// 路径和任务设置
static Route_Task_t Route_Task[] = {
    {.node.x = 5, .node.y = 6, .node.dir = DIR_DOWN, .Task = Start_Task}, // 起始点
    {.node.x = 5, .node.y = 5, .node.dir = DIR_NOTSET, .Task = TFT_Task},
    {.node.x = 3, .node.y = 5, .node.dir = DIR_NOTSET, .Task = NULL},
    {.node.x = 1, .node.y = 5, .node.dir = DIR_DOWN, .Task = NULL},
    {.node.x = 1, .node.y = 3, .node.dir = DIR_DOWN, .Task = NULL},
    {.node.x = 5, .node.y = 3, .node.dir = DIR_DOWN, .Task = NULL},
    {.node.x = 5, .node.y = 1, .node.dir = DIR_DOWN, .Task = NULL},
    {.node.x = 3, .node.y = 1, .node.dir = DIR_DOWN, .Task = End_Task}
};

extern RouteNode CurrentStaus;
extern int8_t RouteTask_Finished[ROUTE_TASK_NUMBER];
extern Route_Task_t Final_Route[ROUTE_TASK_NUMBER * 10];
extern int16_t Final_StepCount;

#endif // _ROUTE_H_
