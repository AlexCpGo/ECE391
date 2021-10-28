#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
// apparrently C does not have boolean data type, so use int for keeping track of true or false

// key input map
unsigned char key_map[NUM_KEYS] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0' /*this is backspace*/, 
    '\0' /*this is tab*/,'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0' /*this is enter*/, '\0' /* this is left ctrl*/, 
    'a', 's','d', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0' /*this is shift*/, '\\', 
    'z', 'x', 'c', 'v','b', 'n', 'm',',', '.', '/', '\0' /*this is shift*/, '*', '\0' /*this is alt*/, ' ', '\0' /*this is CAPS*/
};
//key input map with Shift key pressed
unsigned char key_map_SHIFT[NUM_KEYS] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0' /*this is backspace*/, 
    '\0' /*this is tab*/,'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0' /*this is enter*/, '\0' /* this is left ctrl*/, 
    'A', 'S','D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '\"', '~', '\0' /*this is shift*/, '|',
    'Z', 'X', 'C', 'V','B', 'N', 'M','<', '>', '?', '\0' /*this is shift*/, '*', '\0' /*this is alt*/, ' ', '\0' /*this is CAPS*/
};
// key input map with caps key ON
unsigned char key_map_CAPS[NUM_KEYS] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0' /*this is backspace*/, 
    '\0' /*this is tab*/,'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0' /*this is enter*/, '\0' /* this is left ctrl*/, 
    'A', 'S','D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0' /*this is shift*/, '\\', 
    'Z', 'X', 'C', 'V','B', 'N', 'M',',', '.', '/', '\0' /*this is shift*/, '*', '\0' /*this is alt*/, ' ', '\0' /*this is CAPS*/
};
// key input with CAPS AND SHIFT 
unsigned char key_map_CAPS_SHIFT[NUM_KEYS] = {
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0' /*this is backspace*/, 
    '\0' /*this is tab*/,'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0' /*this is enter*/, '\0' /* this is left ctrl*/, 
    'a', 's','d', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '\"', '~', '\0' /*this is shift*/, '|',
    'z', 'x', 'c', 'v','b', 'n', 'm','<', '>', '?', '\0' /*this is shift*/, '*', '\0' /*this is alt*/, ' ', '\0' /*this is CAPS*/
};

void keyboard_init() {
    enable_irq(KEYBOARD_IRQ_IDX);
    CTRL = 0;
    CAPS = 0;
    SHIFT = 0;
    ALT = 0;
    modifier_key = 0;
    ENTER = 0;
    BACKSPACE = 0;
    //printf("enable_irq successful\n");
}

int update_modifier_key(unsigned char key){
    //if it is a modifier key, return 1, else return 0
    if (key == CAPS_LOCK_KEY){
        if (CAPS == 1){
            CAPS = 0;
            return 1;
        }
        else {
            CAPS = 1;
            return 1;
        }
    }
    else if (key == CTRL_KEY_PRESSED){
        CTRL = 1;
        return 1;
    }
    else if (key == CTRL_KEY_RELEASE){
        CTRL = 0;
        return 1;
    }
    else if (key == LEFT_SHIFT_KEY_PRESSED || key == RIGHT_SHIFT_KEY_PRESSED){
        SHIFT = 1;
        return 1;
    }
    else if (key == LEFT_SHIFT_KEY_RELEASE || key == LEFT_SHIFT_KEY_RELEASE){
        SHIFT = 0;
        return 1;
    }
    else if (key == ALT_KEY_PRESSED){
        ALT = 1;
        return 1;
    }
    else if (key == ALT_KEY_RELEASE){
        ALT = 0;
        return 1;
    }
    else if (key == BACKSPACE_PRESSED){
        BACKSPACE = 1;
        return 0;
    }
    else if (key == BACKSPACE_RELEASE){
        BACKSPACE = 0;
        return 0;
    }
    else{
        return 0;
    }
}

void keyboard_interrupt() {
    cli();
	//while(!inb(KEYBOARD_DATA))
        unsigned char key = inb(KEYBOARD_DATA);
        if (update_modifier_key(key) == 1){ // if a modifier key is pressed (calls modifier key helper function in the process)
            send_eoi(KEYBOARD_IRQ_IDX);
            sti();
            return;
        }
        if (key == ENTER_KEY){
            ENTER = 1;
        }
        // if ctrl+ l is pressed, clear the terminal screen
        else if (CTRL == 1 && key == L_KEY){
            clear();
            int i = 0;
            for (i=0; i<num_bytes; i++){
            	putc(buffer[i]);
			}
        }
        else if (BACKSPACE == 1){
            if(num_bytes > 0){
                backspace(num_bytes);
                buffer[num_bytes] = NULL;
                num_bytes -= 1;
                buffer[num_bytes] = '\0';
            }
        }
        else if (num_bytes == BUFFER_SIZE - 1){
            send_eoi(KEYBOARD_IRQ_IDX);
            sti();
            return;
        }
        else if (CAPS == 1 && SHIFT == 1 && key < NUM_KEYS){
            buffer[num_bytes] = key_map_CAPS_SHIFT[key];
            num_bytes += 1;
            buffer [num_bytes] = '\0';
            putc(key_map[key]);
        }
        //if shift key is held and a key is to be printed
        else if (SHIFT == 1 && key < NUM_KEYS){
            buffer[num_bytes] = key_map_SHIFT[key];
            num_bytes += 1;
            buffer[num_bytes] = '\0';
            putc(key_map_SHIFT[key]);
        }
        // if CAPS lock is on and a key is to be printed
        else if (CAPS == 1 && key < NUM_KEYS){
           buffer[num_bytes] = key_map_CAPS[key];
            num_bytes += 1;
            buffer [num_bytes] = '\0';
            putc(key_map_CAPS[key]);
        }
        else if (CAPS == 1 && SHIFT == 1 && key < NUM_KEYS){
            buffer[num_bytes] = key_map[key];
            num_bytes += 1;
            buffer [num_bytes] = '\0';
            putc(key_map[key]);
        }
        else if (key<NUM_KEYS){
            buffer[num_bytes] = key_map[key];
            num_bytes += 1;
            buffer [num_bytes] = '\0';
            putc(key_map[key]);
	    }
        //TODO: HANDLE BACKSPACE
    send_eoi(KEYBOARD_IRQ_IDX);
    sti();
    return;
}




