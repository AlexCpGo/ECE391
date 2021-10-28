#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "lib.h"
#include "types.h"
#include "keyboard.h"

#define BUFFER_SIZE 128
#define VGA_WIDTH 80

extern int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open_terminal(const uint8_t * filename);
extern int32_t close_terminal(int32_t fd);


extern void terminal_init();
extern void clear_buffer();
extern void clear_buffer_screen(void* buf);
extern void set_cursor(int x, int y);
//extern void update_cursor(int x, int y);

uint8_t buffer[BUFFER_SIZE];
int num_bytes;
int char_printed;
int counter;
int flag;
#endif

//scrolling is a for loop of memcpy 
// 25 rows, 80 columns
// then clear the bottom line

//BACKSPACE
// keyboard buffer need to have the character wiped
//screen position changed
//replace with blank character
//handle with new line as well
//handle when it is at the end (size is already 0)
//need to handle wrapping as well
