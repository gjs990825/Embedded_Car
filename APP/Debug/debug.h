#if !defined(__DEBUG_H_)
#define __DEBUG_H_

#include "sys.h"

#define DEBUG_PIN_1 PFout(8)
#define DEBUG_PIN_2 PHout(9)
#define DEBUG_PIN_3 PFout(9)

void DebugPin_Init(void);
void DebugTimer_Init(uint16_t arr, uint16_t psc);
void print_info(char *str, ...);


#endif // __DEBUG_H_
