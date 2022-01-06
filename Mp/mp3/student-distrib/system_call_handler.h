#ifndef SYSTEM_CAL_HANDLER_H
#define SYSTEM_CALL_HANDLER_H

#include    "make_system_call.h"

#ifndef     ASM
# a function to handle the system call in ASM
extern void handle_system_call();

#endif
