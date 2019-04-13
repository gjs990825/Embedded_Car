#ifndef __TIMER_H__
#define __TIMER_H__

#include "sys.h"

// �����Ƿ�ʱ����λms��
#define IsTimeOut(setTimeStamp, timeOutLimit) (global_times > (setTimeStamp + timeOutLimit))
// ��ȡ��ǰʱ���
#define Get_GlobalTimeStamp() (global_times)

extern volatile uint32_t global_times;

void Timer_Init(uint16_t arr, uint16_t psc);

// ���������Ҫ�ķ�ʱ�䣬ʹ�ú궨���������������ʱ�俪��
// warning : Ŀǰû�н������ݽ��ղ�����������Ҫ��֤�˸Ķ��Խ��յ�Ӱ��

// ͨ��ʱ�����ʱ������1ms��
static inline void delay(uint16_t ms)
{
    uint32_t startStamp = Get_GlobalTimeStamp();
    for(;;)
    {
        if (IsTimeOut(startStamp, ms))
            break;
    }
}

// uint32_t gt_get_sub(uint32_t c);
static inline uint32_t gt_get_sub(uint32_t c)
{
    (c > global_times) ? (c -= global_times) : (c = 0);
    return c;
}

// uint32_t gt_get(void);
#define gt_get Get_GlobalTimeStamp

#endif
