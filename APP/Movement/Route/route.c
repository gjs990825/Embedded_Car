#include "route.h"
#include "string.h"

// 主车当前位置状态
RouteNode CurrentStaus;
// 主车下一个位置和状态
RouteNode NextStatus;
// 主车任务完成情况
int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};

