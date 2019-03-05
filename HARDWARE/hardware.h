#if !defined(_HARDWARE_H_)
#define _HARDWARE_H_

#include "sys.h"
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "syn7318.h"
#include "power_check.h"
#include "can_user.h"
#include "data_base.h"
#include "roadway_check.h"
#include "tba.h"
#include "data_base.h"
#include "swopt_drv.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "delay.h"
#include "can_user.h"
#include "Timer.h"
#include "Rc522.h"
#include "malloc.h"
#include "a_star.h"
#include "debug.h"
#include "movement.h"

typedef enum
{
    TRACKING = 0x00,        // 循迹状态
    CROSSROAD = 0x01,       // 十字路口
    TURNCOMPLETE = 0x02,    // 转弯完成
    FORBACKCOMPLETE = 0x03, // 前进后退完成(定长循迹)
    OUTTRACK = 0x04         // 出线
} StopFlag_t;


void Hardware_Init(void);

#endif // _HARDWARE_H_
