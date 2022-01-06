#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "types.h"

#define TERMINAL_BUF_SIZE 128
#define NUM_TERMINALS 3
#define INIT_BLANK -1

// struct for the terminal and buffer information
typedef struct terminal {
  int32_t id;
  int32_t pid;  // pid of running program

  int32_t cx, cy;  // cursor position
  char* vid_mem;   // video mem backup location

  int enter_pressed;  // enter pressed signal from kbd
  char buf[TERMINAL_BUF_SIZE];
  int tot_running_process;
  uint8_t buf_pos;

  uint32_t rtc_frequency;
} terminal_t;

extern terminal_t* cur_terminal;
extern terminal_t* running_terminal;
extern terminal_t terminals[];

void terminal_init();

// open terminal
int32_t open_terminal(const uint8_t* filename);

// close a terminal
int32_t close_terminal(int32_t fd);

// read the terminal input
int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes);

// write to the terminal
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes);

// clear the contents of the buffer
void clear_buffer(terminal_t* term);

// switch to terminal
void switch_to_terminal(int32_t id);

#endif
