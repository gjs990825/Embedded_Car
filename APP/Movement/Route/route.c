#include "route.h"
#include "string.h"

// ������ǰλ��״̬
RouteNode CurrentStaus;
// ������һ��λ�ú�״̬
RouteNode NextStatus;
// ��������������
int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};

