#ifndef __TIMER_H__
#define __TIMER_H__

// �����Ƿ񳬹��趨ʱ�䣨��λms��
#define Check_IsTimeOut(setTimeStamp, timeOutLimit) (global_times < (setTimeStamp + timeOutLimit))
#define Get_GlobalTimeStamp() (global_times)

extern volatile uint32_t global_times;
extern volatile uint32_t delay_ms_const;

void Timer_Init(uint16_t arr, uint16_t psc);
uint32_t gt_get_sub(uint32_t c);
uint32_t gt_get(void);

#endif
