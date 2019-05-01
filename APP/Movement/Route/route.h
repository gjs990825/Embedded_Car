#if !defined(_ROUTE_H_)
#define _ROUTE_H_

#include "sys.h"

// 坐标点信息
typedef struct RouteNode_Struct
{
	int8_t x;
	int8_t y;
	int8_t dir; // 用到不多，需要精简
} RouteNode_t;

// 方向定义
typedef enum
{
	DIR_NOTSET = 0,
	DIR_UP,
	DIR_LEFT,
	DIR_DOWN,
	DIR_RIGHT
} Direction_t;

// 设定任务点（转换字符串生成坐标信息）
typedef struct RouteSetting_Struct
{
	uint8_t coordinate[2]; // 字符串坐标
	RouteNode_t node;	  // 坐标点数据
	void (*Task)(void);	// 任务函数指针
} RouteSetting_t;

// 任务设定和任务个数
extern RouteSetting_t Route_Task[];
extern uint8_t ROUTE_TASK_NUMBER;

// 寻卡测试用路径
extern RouteSetting_t RFID_TestRoute[];
extern uint8_t RFID_TESTROUTE_NUMBER;

extern RouteNode_t CurrentStaus;
extern RouteNode_t NextStatus;

// 坐标字符串转换

RouteNode_t Coordinate_Covent(uint8_t str[2]);
uint8_t *ReCoordinate_Covent(int8_t x, int8_t y);

// 坐标生成

bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count);

// 坐标信息提取和处理

int8_t Get_TaskNumber(uint8_t coordinate[2], uint8_t *route, uint8_t nTimes);
bool RouteString_Process(uint8_t *prefix, uint8_t *route, uint8_t *buffer);

#endif // _ROUTE_H_
