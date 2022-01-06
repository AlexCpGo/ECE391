#ifndef __PIT_H__
#define __PIT_H__

#include "i8259.h"

#define PIT_PORT_DATA0 0x40
#define PIT_PORT_CMD 0x43

#define PIT_CMD 0x36

#define PIT_MAX_FREQ 1193180
#define PIT_FREQ (PIT_MAX_FREQ / 20)

void pit_init();

void pit_interrupt();

#endif
