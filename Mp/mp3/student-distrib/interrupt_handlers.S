#define ASM 1

#include "interrupt_handlers.h"

.globl handle_keyboard
.globl handle_rtc
.globl handle_pit

handle_keyboard:
    pushal
    pushfl
    call keyboard_interrupt
    popfl
    popal
    iret

handle_rtc:
    pushal                          
    pushfl
    call rtc_interrupt                  
    popfl              
    popal                           
    iret   

handle_pit:
    pushal                          
    pushfl
    call pit_interrupt
    popfl              
    popal                           
    iret   

