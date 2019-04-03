#ifndef __CBA_H
#define __CBA_H

#include "stm32f4xx.h"
#include "sys.h"

// ∞¥º¸…Ë÷√∆Ù∂Ø≥Ã–Ú

#include "infrared.h"
#include "data_base.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "data_base.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"
#include "voice.h"
#include "data_from_host.h"
#include "debug.h"
#include "bh1750.h"

// ∞¥º¸≈‰÷√
#define S1 PIin(4)
#define S2 PIin(5)
#define S3 PIin(6)
#define S4 PIin(7)

// LED≈‰÷√
#define LED1 PHout(12)
#define LED2 PHout(13)
#define LED3 PHout(14)
#define LED4 PHout(15)

//∑‰√˘∆˜
#define MP_SPK PHout(5)

void Cba_Init(void);
void Beep(uint8_t times);
void KEY_Check(void);

#endif
