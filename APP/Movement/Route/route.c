#include "route.h"
#include "string.h"

// ��ǰλ�ú�״̬
RouteNode CurrentStaus;
// ����������
int8_t RouteTask_Finished[ROUTE_TASK_NUMBER] = {0};
// ÿ���������10���� * 10
Route_Task_t Final_Route[ROUTE_TASK_NUMBER * 10];
// ·������
int16_t Final_StepCount = 0;
