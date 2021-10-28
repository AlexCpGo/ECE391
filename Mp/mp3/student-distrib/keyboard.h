#include "types.h"

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define NUM_KEYS 80
#define KEYBOARD_DATA 0x60
#define CAPS_LOCK_KEY 0x3A
#define CTRL_KEY_PRESSED 0x1D
#define CTRL_KEY_RELEASE 0x9D
#define LEFT_SHIFT_KEY_PRESSED 0x2A
#define LEFT_SHIFT_KEY_RELEASE 0xAA
#define RIGHT_SHIFT_KEY_PRESSED 0x36
#define RIGHT_SHIFT_KEY_RELEASE 0xB6
#define ALT_KEY_PRESSED 0x38
#define ALT_KEY_RELEASE 0xB8

#define BACKSPACE_PRESSED 0x0E
#define BACKSPACE_RELEASE 0x8E
#define L_KEY 0x26
#define ENTER_KEY 0x1C


extern void keyboard_init();
extern void keyboard_interrupt();
int update_modifier_key(unsigned char key);

// apparrently C does not have boolean data type, so use int for keeping track of true or false
int CTRL;
int CAPS;
int SHIFT;
int ALT;
int modifier_key;
int ENTER;
int BACKSPACE;


#endif
