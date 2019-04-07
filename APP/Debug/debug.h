#if !defined(__DEBUG_H_)
#define __DEBUG_H_

#include "sys.h"

#define DEBUG_PIN_1 PFout(8)
#define DEBUG_PIN_2 PHout(9)
#define DEBUG_PIN_3 PFout(9)

#define DEBUG_PIN_1_SET() DEBUG_PIN_1 = 1
#define DEBUG_PIN_1_RESET() DEBUG_PIN_1 = 0
#define DEBUG_PIN_2_SET() DEBUG_PIN_2 = 1
#define DEBUG_PIN_2_RESET() DEBUG_PIN_2 = 0
#define DEBUG_PIN_3_SET() DEBUG_PIN_3 = 1
#define DEBUG_PIN_3_RESET() DEBUG_PIN_3 = 0

void DebugPin_Init(void);
void DebugTimer_Init(uint16_t arr, uint16_t psc);
void print_info(char *str, ...);


#endif // __DEBUG_H_
