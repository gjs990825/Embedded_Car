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
    {.node.x = 5, .node.y = 5, .node.dir = DIR_NOTSET, .Task = NULL}, // Task_5_5
    {.node.x = 3, .node.y = 5, .node.dir = DIR_NOTSET, .Task = NULL}, // Task_3_5
    {.node.x = 1, .node.y = 5, .node.dir = DIR_NOTSET, .Task = NULL}, // Task_1_5
    {.node.x = 1, .node.y = 3, .node.dir = DIR_NOTSET, .Task = Task_1_3}, // Task_1_3
    {.node.x = 5, .node.y = 3, .node.dir = DIR_NOTSET, .Task = Task_5_3}, // Task_5_3
    {.node.x = 5, .node.y = 2, .node.dir = DIR_NOTSET, .Task = Task_5_2}, // Task_5_2
    {.node.x = 5, .node.y = 1, .node.dir = DIR_NOTSET, .Task = Task_5_1}, // Task_5_1
    {.node.x = 3, .node.y = 1, .node.dir = DIR_NOTSET, .Task = Task_3_1}
};

// 当前位置状态
extern RouteNode CurrentStaus;
// 任务完成情况
extern int8_t RouteTask_Finished[ROUTE_TASK_NUMBER];



// 储存运算出的路径
extern Route_Task_t Final_Route[ROUTE_TASK_NUMBER * 10];
// 记录当前路径步数
extern int16_t Final_StepCount;


#endif // _ROUTE_H_
