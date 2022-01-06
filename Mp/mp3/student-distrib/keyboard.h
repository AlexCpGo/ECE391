#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define NUM_KEYS 80
#define KEYBOARD_DATA 0x60

// special key codes
#define MODIFIER 0x80

#define BACKSPACE 0x0E
#define TAB 0x0F
#define ENTER 0x1C

#define CTRL_DOWN 0x1D
#define LSHIFT_DOWN 0x2A
#define RSHIFT_DOWN 0x36
#define ALT_DOWN 0x38

#define SPACE_DOWN 0x39

#define CAPS_LOCK 0x3A
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

#define CTRL_UP (CTRL_DOWN | MODIFIER)
#define LSHIFT_UP (LSHIFT_DOWN | MODIFIER)
#define RSHIFT_UP (RSHIFT_DOWN | MODIFIER)
#define ALT_UP (ALT_DOWN | MODIFIER)

void keyboard_init();
void keyboard_interrupt();

#endif
