#if !defined(__PID_H_)
#define __PID_H_

#include "sys.h"

// 计算出的PID值
extern float PID_value;

// 根据当前误差计算
void Calculate_pid(int inputError);
// 清空PID数据
void PidData_Clear(void);



#endif // __PID_H_
