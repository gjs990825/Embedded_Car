#if !defined(__PID_H_)
#define __PID_H_

#include "sys.h"

// �������PIDֵ
extern float PID_value;

// ���ݵ�ǰ������
void Calculate_pid(int inputError);
// ���PID����
void PidData_Clear(void);



#endif // __PID_H_
