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

// 循迹状态
#define TRACKING 0x00
// 十字路口
#define CROSSROAD 0x01
// 转弯完成
#define TURNCOMPLETE 0x02
// 前进后退完成
#define FORBACKCOMPLETE 0x03
// 出线
#define OUTTRACK 0x04


void Hardware_Init(void);


#endif // _HARDWARE_H_

