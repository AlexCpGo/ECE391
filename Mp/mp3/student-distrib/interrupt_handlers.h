#ifndef __INTERRUPT_HANDLER_H__
#define __INTERRUPT_HANDLER_H__

#ifndef ASM

//extern void handle_rtc();
extern void handle_keyboard();
extern void handle_rtc();
extern void handle_pit();

#endif  // ASM

#endif  // __INTERRUPT_HANDLER_H__
