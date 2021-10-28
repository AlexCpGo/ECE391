#include "terminal.h"

void terminal_init(){
    num_bytes = 0;
    clear();
    clear_buffer();
    counter = 0;
    flag = 0;
    //update_cursor(0,0);
}

int32_t open_terminal(const uint8_t* filename){
    return 0;
}

int32_t close_terminal(int32_t fd){
    return 0;
}

int32_t read_terminal(int32_t fd, void* buf, int32_t nbytes){
    // keep spinning until is pressed
    // when enter ir pressed, take what is in keyboard buffer and put into buffer in argument
        // use memcpy
    //user is trying to communicate with system 
    // user just typed and pressed something
        while(ENTER==0);
		
		if (buf == NULL) return 0;
		
		if (ENTER == 1){
            memcpy(buf, buffer, num_bytes);
            //clear_buffer();
            //clear_buffer_screen(buf);
            num_bytes = 0;
            ENTER = 0;
            putc('\n');
            counter = 0;
            flag = 1;
            
    		int len = nbytes < BUFFER_SIZE ? nbytes : BUFFER_SIZE;
    		char* c_buf = buf;
    		for (counter = 0; counter < len; counter++) {
        		c_buf[counter] = buffer[counter];
        		if (c_buf[counter] == '\n') {
            		counter++;
            		break;
        		}
    		}
    		c_buf[counter] = '\0';
    		clear_buffer();
        }
    return counter;
}


int32_t write_terminal(int32_t fd, const void* buf, int32_t nbytes){
    while (((char*)buf)[counter] != '\0'){
        putc(((char*)buf)[counter]);
        counter++;
    }
    if(flag == 1){
        clear_buffer_screen((void *)buf);
        putc('\n');
        flag = 0;
    }
    return 0;
}
//need to clear buffer here
void clear_buffer(){
    int x;
    for (x = 0; x < 128; x++){
        buffer[x] = NULL;
    }
    buffer[0] = '\0'; 
}
void clear_buffer_screen(void* buf){
    int x;
    for (x = 0; x < 128; x++){
        ((char*)buf)[x] = NULL;
    }
    ((char*)buf)[0] = '\0';
}


//TODO:
//scroll
