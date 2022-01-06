#include "keyboard.h"
#include "terminal.h"
#include "lib.h"
#include "i8259.h"

// keyboard status
#define NUM_KBD_STATUS 4

// key input map
unsigned char key_map[NUM_KBD_STATUS][NUM_KEYS] = {
    // default
    {
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'
    },
    // Caps enabled
	{
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'
    },
    // Shift down
	{
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'
    },
    // Caps and shift
	{
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'
    }
};

// key status
typedef enum key_status {
    RELEASED,
    PRESSED
} key_status_t;

key_status_t caps_status = RELEASED;
key_status_t ctrl_status = RELEASED;
key_status_t shift_status = RELEASED;
key_status_t alt_status = RELEASED;

// helper functions
void modify_kbd_status(int key);
void handle_alpha_numeric_keys(int key);
/* Description: initializes the keyboard
* int isPowerOfTwo()
* inputs: NONE
* return value: NONE
* EFFECTS: connects the keyboard to he PIC
*/
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ_IDX);
}
/* Description: handles keyboard interrupt
* inputs: NONE
* return value: NONE
* EFFECTS: handles keyboard inputs and updates the buffer
*/
void keyboard_interrupt() {
    cli();

    int key = inb(KEYBOARD_DATA);
    if (0 < key && key < NUM_KEYS) { // alphanumeric and special key
        switch (key) {
            case BACKSPACE:
                if (cur_terminal->buf_pos > 0) {
                    cur_terminal->buf[--cur_terminal->buf_pos] = '\0';
                    backspace();
                }
                break;
            case TAB: {
                int i;
                for (i = 0; i < 4; i++)
                    handle_alpha_numeric_keys(SPACE_DOWN);
                break;
            }
            case ENTER:
                if (!cur_terminal->enter_pressed) {
                    cur_terminal->buf[cur_terminal->buf_pos++] = '\n';
                    cur_terminal->enter_pressed = PRESSED;
                    putc('\n');
                }
                break;
            case CTRL_DOWN:
            case LSHIFT_DOWN:
            case RSHIFT_DOWN:
            case ALT_DOWN:
            case CAPS_LOCK:
                modify_kbd_status(key);
                break;
            default:
                handle_alpha_numeric_keys(key);
        }
    } else {  // handle the rest
        modify_kbd_status(key);
    }

    send_eoi(KEYBOARD_IRQ_IDX);
    sti();
}
/* Description: handles special keys pressed on the keyboard
* inputs: NONE
* return value: NONE
* EFFECTS: changes status flags
*/
void modify_kbd_status(int key) {
    switch (key) {
        case CAPS_LOCK:
            caps_status = ~caps_status;
            break;
        case CTRL_DOWN:
            ctrl_status = PRESSED;
            break;
        case CTRL_UP:
            ctrl_status = RELEASED;
            break;
        case LSHIFT_DOWN:
        case RSHIFT_DOWN:
            shift_status = PRESSED;
            break;
        case LSHIFT_UP:
        case RSHIFT_UP:
            shift_status = RELEASED;
            break;
        case ALT_DOWN:
            alt_status = PRESSED;
            break;
        case ALT_UP:
            alt_status = RELEASED;
            break;
    }
}
/* Description: helper function for alphanumeric keys pressed on the keyboard
* inputs: NONE
* return value: NONE
* EFFECTS: updates the buffer and terminal based on what is typed
*/
void handle_alpha_numeric_keys(int key) {
    // compute kbd status
    int kbd_status = caps_status | (shift_status << 1);
    char c = key_map[kbd_status][key];

    if (ctrl_status == PRESSED) {
        if (c == 'l' || c == 'L') {
            int screen_y = get_screen_y();
            scroll_up_by(screen_y);
        }
    } else if (alt_status == PRESSED) {
        sti();
        if (key == F1)
            switch_to_terminal(0);
        else if (key == F2)
            switch_to_terminal(1);
        else if (key == F3)
            switch_to_terminal(2);
    } else {
        if (cur_terminal->buf_pos < TERMINAL_BUF_SIZE-1) {
            cur_terminal->buf[cur_terminal->buf_pos++] = c;
            putc(c);
        }
    }
}
