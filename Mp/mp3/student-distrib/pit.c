#include "pit.h"
#include "lib.h"
#include "scheduler.h"

// Ref: https://wiki.osdev.org/Programmable_Interval_Timer
/*
Initializes PIT device
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables irq for PIT
*/
void pit_init() {
    // set PIT mode
    outb(PIT_CMD, PIT_PORT_CMD);
    // set frequency
    outb(0xff & PIT_FREQ, PIT_PORT_DATA0);
    outb(PIT_FREQ >> 8, PIT_PORT_DATA0);
    // enable irq
    enable_irq(PIT_irq_num);
}

void pit_interrupt() {
    send_eoi(PIT_irq_num);
    schedule();
}
