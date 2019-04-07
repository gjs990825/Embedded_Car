#ifndef __TIMER_H__
#define __TIMER_H__

// 返回是否超时（单位ms）
#define Check_IsTimeOut(setTimeStamp, timeOutLimit) (global_times > (setTimeStamp + timeOutLimit))
// 获取当前时间戳
#define Get_GlobalTimeStamp() (global_times)

extern volatile uint32_t global_times;

void Timer_Init(uint16_t arr, uint16_t psc);

// 程序调用需要耗费时间，使用宏定义和内联函数减少时间开销
// warning : 目前没有进行数据接收操作，后续需要验证此改动对接收的影响

// uint32_t gt_get_sub(uint32_t c);
static inline uint32_t gt_get_sub(uint32_t c)
{
    (c > global_times) ? (c -= global_times) : (c = 0);
    return c;
}

// uint32_t gt_get(void);
#define gt_get Get_GlobalTimeStamp

#endif
