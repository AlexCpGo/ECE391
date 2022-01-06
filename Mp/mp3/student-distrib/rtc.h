#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

#define RTC_PORT   0x70
#define RTC_DATA   0x71

#define R_A 0x8A
#define R_B 0x8B
#define R_C 0x8C
#define IRQ8    8

#define RTC     0x70
#define CMOS    0x71

#define MIN_FREQ 2
#define MAX_FREQ 1024

#define RTC_MASK 0xF0



/* Initialize the RTC*/
uint32_t rtc_init();
// void turn_on_irq_8();
int32_t rtc_rate(int32_t freq);
extern void rtc_interrupt();
extern int32_t rtc_open(const uint8_t* filename);
extern int32_t rtc_close(int32_t fd);
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);

#endif

