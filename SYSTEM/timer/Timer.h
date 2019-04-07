#ifndef __TIMER_H__
#define __TIMER_H__

// �����Ƿ�ʱ����λms��
#define Check_IsTimeOut(setTimeStamp, timeOutLimit) (global_times > (setTimeStamp + timeOutLimit))
// ��ȡ��ǰʱ���
#define Get_GlobalTimeStamp() (global_times)

extern volatile uint32_t global_times;

void Timer_Init(uint16_t arr, uint16_t psc);

// ���������Ҫ�ķ�ʱ�䣬ʹ�ú궨���������������ʱ�俪��
// warning : Ŀǰû�н������ݽ��ղ�����������Ҫ��֤�˸Ķ��Խ��յ�Ӱ��

// uint32_t gt_get_sub(uint32_t c);
static inline uint32_t gt_get_sub(uint32_t c)
{
    (c > global_times) ? (c -= global_times) : (c = 0);
    return c;
}

// uint32_t gt_get(void);
#define gt_get Get_GlobalTimeStamp

#endif
