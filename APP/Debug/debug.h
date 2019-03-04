#if !defined(__DEBUG_H_)
#define __DEBUG_H_

#include "sys.h"

// #define printf(x) print_info(x)
void DebugTimer_Init(uint16_t arr, uint16_t psc);
void print_info(char *str, ...);


#endif // __DEBUG_H_
