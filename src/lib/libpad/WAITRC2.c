/* LIBPAD's short busy-wait timer over root counter 2 (wait is in system-clock/8 ticks). */
#include "pad_int.h"
#include "psx_hw.h"

unsigned long _startTime;
unsigned long _waitTime;

void setRC2wait(unsigned long wait)
{
    _waitTime = wait;
    _startTime = *(volatile unsigned short *)RCNT2_COUNT_ADDR;
}

int chkRC2wait(void)
{
    unsigned long now = *(volatile unsigned short *)RCNT2_COUNT_ADDR;

    if (now < _startTime) {
        if (*(volatile unsigned short *)RCNT2_TARGET_ADDR != 0) {
            now += *(volatile unsigned short *)RCNT2_TARGET_ADDR;
        } else {
            now += 0x10000;
        }
    }
    if (*(volatile unsigned short *)RCNT2_MODE_ADDR & RCNT_MODE_DIV8_CLOCK) {
        return (now - _startTime) >= _waitTime;
    }
    return ((now - _startTime) >> 3) >= _waitTime;
}