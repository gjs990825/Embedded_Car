#include "route.h"
#include "string.h"

// 当前位置和状态
RouteNode CurrentStaus;
// 任务完成情况
int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};
// 每个任务最多10个点 * 10
Route_Task_t Final_Route[ROUTE_TASK_NUMBER * 10];
// 路径计数
int16_t Final_StepCount = 0;
