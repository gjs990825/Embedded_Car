#include "debug.h"
#include "stdarg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canp_hostcom.h"

void print_info(char *str, ...)
{
    uint16_t len;
    uint8_t buf[50];
    va_list ap;
    va_start(ap, str);
    vsprintf((char *)buf, str, ap);
    va_end(ap);
    len = strlen((char *)buf);
    Send_InfoData_To_Fifo(buf, len);
}

