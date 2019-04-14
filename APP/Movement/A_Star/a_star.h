#if !defined(_A_STAR_H_)
#define _A_STAR_H_

#include "sys.h"
#include "route.h"

bool A_Star_GetTestRoute(RouteNode_t current, RouteNode_t next, RouteNode_t *finalRoute, uint8_t *routeCount);

#endif // _A_STAR_H_
