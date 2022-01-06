#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "page.h"
#include "rtc.h"
#include "system_execute_c.h"
#include "scheduler.h"

// give it garbage value
terminal_t* cur_terminal;
terminal_t* running_terminal;
terminal_t terminals[NUM_TERMINALS];

/*DESCRIPTION 
*   this function initializes the Terminal
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= clears the buffer, clears the screen and set the flags
*/
void terminal_init() {
    int i;
    memset(terminals, 0, sizeof(terminal_t) * NUM_TERMINALS);
    for (i = 0; i < NUM_TERMINALS; i++) {
        terminals[i].id = i;
        terminals[i].pid = INIT_BLANK;
        terminals[i].rtc_frequency = MAX_FREQ;
        terminals[i].vid_mem = (char*)(VID_ADDR + SIZE_4KB * (i+1));
        terminals[i].tot_running_process = 0;
        clear_mem(terminals[i].vid_mem);
    }

    clear();
    cur_terminal = &terminals[0];

    set_screen_pos(cur_terminal->cx, cur_terminal->cy);
}

/*DESCRIPTION 
*   this function opens the terminal
* INPUT = a ptr to the file
* OUTPUT = 0 on success
* SIDE EFFECTS= opens the terminal
*/
int32_t open_terminal(const uint8_t* filename) {
    return 0;
}

/*DESCRIPTION 
*   this function closes the terminal
* INPUT = file descriptor
* OUTPUT = 0 on success
* SIDE EFFECTS= closes the terminal
*/
int32_t close_terminal(int32_t fd) {
    return 0;
}


/*DESCRIPTION 
*   this function reads the value in the buffer and handles the case for when an enter key is pressed
* INPUT = file_descriptor, pointer to the buffer of terminal, and the number of bytes being read
* OUTPUT = returns the number of bytes
* SIDE EFFECTS= establishes the connection from the RTC with the PIC
*/
int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes) {
    while (running_terminal->enter_pressed == 0)
        ;

    if (buf == NULL)
        return 0;
    cli();    
    int i;
    int len = nbytes < TERMINAL_BUF_SIZE ? nbytes : TERMINAL_BUF_SIZE;
    char* c_buf = buf;
    for (i = 0; i < len; i++) {
        c_buf[i] = running_terminal->buf[i];
        if (c_buf[i] == '\n') {
            i++;
            break;
        }
    }
    c_buf[i] = '\0';
    clear_buffer(running_terminal);
    sti();
    return i;
}

/*DESCRIPTION 
*   this function writes unto the terminal
* INPUT = file_descriptor, ptr to the buffer for the terminal, and the number of bytes being read
* OUTPUT = returns 0 on success
* SIDE EFFECTS= writes the information on the buffer unto the screen
*/
int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL || nbytes == 0)
        return 0;

    cli();
    int i;
    int cnt = 0;
    char* c_buf = (char*)buf;
    if (running_terminal == cur_terminal) {
        for (i = 0; i < nbytes; i++) {
            if (c_buf[i] != '\0') {
                putc(c_buf[i]);
                cnt++;
            }
        }
    } else {
        for (i = 0; i < nbytes; i++) {
            if (c_buf[i] != '\0') {
                putc_at_term(c_buf[i], running_terminal);
                cnt++;
            }
        }
    }
    sti();
    return cnt;
}

/*DESCRIPTION 
*   this function clears the keybaord buffer
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= clears the buffer for the keyboard and reset it to be a null as the first character
*/
void clear_buffer(terminal_t* term) {
    memset(term->buf, 0, TERMINAL_BUF_SIZE);
    term->buf_pos = 0;
    term->enter_pressed = 0;
}

/*DESCRIPTION 
*   this function launches terminal with `id`
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= execute shell for newly initiated terminal
*/
// void launch_terminal(int32_t id) {
//     cur_terminal = &terminals[id];
//     memcpy((void*)VID_ADDR, cur_terminal->vid_mem, NUM_COLS * NUM_ROWS * 2);
//     set_screen_pos(cur_terminal->cx, cur_terminal->cy);
//     if (cur_terminal->pid == -1) {
//         asm volatile(
//             "movl %%ebp, %0;"
//             "movl %%esp, %1"
//             :"=r"(cur_process->ebp), "=r"(cur_process->esp)
//         );
//         cur_process = NULL;
//         sti();
//         execute((uint8_t*)"shell");
//     } else {
//         // cur_process = get_pcb_ptr(cur_terminal->pid);
//     }
// }

/*DESCRIPTION 
*   this function swithces the current terminal to terminal of id
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= inits new terminal
*/
void switch_to_terminal(int32_t id) {
    if (cur_terminal->id == id)
        return;
        
    cli();

    // save current terminal info
    cur_terminal->cx = get_screen_x();
    cur_terminal->cy = get_screen_y();
    memcpy(cur_terminal->vid_mem, (void*)VID_ADDR, SIZE_4KB);//NUM_COLS * NUM_ROWS * 2

    // running_terminal = &terminals[id];
    cur_terminal = &terminals[id];
    memcpy((void*)VID_ADDR, cur_terminal->vid_mem, SIZE_4KB);
    set_screen_pos(cur_terminal->cx, cur_terminal->cy);
    if (cur_terminal->pid == INIT_BLANK) {
        asm volatile(
            "movl %%ebp, %0;"
            "movl %%esp, %1"
            :"=r"(cur_process->ebp), "=r"(cur_process->esp)
        );
        cur_process = NULL;
        sti();
        execute((uint8_t*)"shell");
    }
    sti();
    // cur_process = get_pcb_ptr(cur_terminal->pid);

    
}
