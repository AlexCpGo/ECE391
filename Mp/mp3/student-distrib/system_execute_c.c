#include "system_execute_c.h"
#include "rtc.h"
#include "terminal.h"
#include "fs.h"
#include "idt.h"
#include "scheduler.h"
// 6 processes, can be active
// maybe put this in PCB instead?
// exception_flag = 0;
uint8_t valid_files[MAGIC_NUMBER] = {0x7F, 0x45, 0x4C, 0x46}; // these are magic numbers as descibed in the OH slides
// uint8_t num_active_pcb = 0; // global variable to keep track of number of active processes
int processes_array[MAX_NUM_PROCESS] = {0, 0, 0, 0, 0, 0};
PCB* cur_process = NULL;

/*DESCRIPTION 
*   this function is called everytime a system call for execute is being called
* INPUT = command -- parameter for what syscall it should be
* OUTPUT = returns 0;
* SIDE EFFECTS= establishes the proper paging unto VMEM and PHYMEM as well as PCB that holds the information
*/
int32_t execute(const uint8_t * command){
    /**************** SANITY CHECK ***************/
    if(command == NULL){
        return FAIL;
    }
    cli();
    // allocate pid
    int pid = 0;
    int i, j;

    // allocate buffers
    uint8_t file_buffer[MAX_arglen];
    int8_t args_buffer[MAX_arglen];
    if(cur_terminal->tot_running_process >= 4){
        printf("Max processes already \n");
        return FAIL;}

    /************************************/
    /********* PARSE ARGUMENTS *********/
    /************************************/

    for (i = 0; i< MAX_arglen; i++){
        file_buffer[i] = '\0';
    }

    // skip first few spaces
    i = 0;
    while(command[i] == ' '){
        i++;
        if (command[i] == '\0')
            break;
    }

    // get command
    j = 0;
    while(command[i] != ' ' && command[i] != '\0'){
        file_buffer[j] = command[i];
        i++;
        j++;
    }

    // skip spaces between command and args
    while (command[i] == ' ') {
        i++;
        if (command[i] == '\0')
            break;
    }

    // get the args
    strncpy(args_buffer, (int8_t*)(&command[i]), MAX_arglen);


    /****************************************/
    /********* CHECK FOR EXECUTABLE *********/
    /****************************************/

    unsigned char check_exec_buffer[CHECK_EXECUTABLE_BUFF_LEN];
    // check if file exist in the final systems
    if(file_open(file_buffer) == FAIL){
        // if not available executable, return -1
        printf("file does not exist in system \n");
        sti();
        return FAIL;
    }
    // if dentry is error
    dentry_t program_dentry;
    if( read_dentry_by_name(file_buffer, &program_dentry) == FAIL){
        printf("dentry is error \n");
        sti();
        return FAIL;
    }
    // if read_data errors then ret FAIL
    if(read_data(program_dentry.inode_idx, 0, check_exec_buffer, CHECK_EXECUTABLE_BUFF_LEN) == FAIL){
        printf("fail to read data \n");
        sti();
        return FAIL;
    }
    // check if first four bytes are what we are looking for (the magic numbers ELF)
    for(i = 0; i < 4; i++){
        int flag = 0;
        // check if first four bytes is within the list of bytes for executable
        for(j = 0; j < 4; j++){
            if(check_exec_buffer[i] == valid_files[j]){
                flag = 1;
            }
        }
        // doesnt match with any so just return FAIL
        if(flag == 0){
            printf("not magic number \n");
            sti();
            return FAIL;
        }
        //printf("is a magic number \n");
    }

    // check for available process
    for(pid = 0; pid < MAX_NUM_PROCESS; pid++){
        if(processes_array[pid] == INACTIVE){
            processes_array[pid] = ACTIVE;
            break;
        }
    }

    if (pid == MAX_NUM_PROCESS) {
        printf("Max processes already \n");
        sti();
        return FAIL;
    }

    cur_terminal->tot_running_process++;
    /*********************************/
    /********* SET UP PAGING *********/
    /*********************************/
    // map the virtual memory for process to the corresponding physical memory
    // virtual memory (128MB) -> 8MB + pid * 4MB
    map_4mb_to_pd(SIZE_128MB, SIZE_8MB + pid * SIZE_4MB);

    /*****************************************/
    /********* LOAD FILE INTO MEMORY *********/
    /*****************************************/
    // set current length  to a very big number, not sure yet how big do I want it, check again later
    read_data(program_dentry.inode_idx, 0, (uint8_t *)program_image_addr, 1000000);

    /**********************************************/
    /********* CREATE PCB FOR NEW PROCESS *********/
    /**********************************************/
    uint8_t EIP_BUFF[4]; // eip is 32 bits hence 4 element long
    read_data(program_dentry.inode_idx, 24, EIP_BUFF, 4);// eip is 32 bits hence 4 element long, 24 because it starts from the 24th byte
    uint32_t temp_ebp;
    uint32_t temp_esp;

    //declare the PCB ptr
    PCB* curr_PCB = get_pcb_ptr(pid);
    //iniitalize elements in PCB
    curr_PCB->process_ID = pid;
    if (cur_process == NULL){// if it is the first process
        curr_PCB->parent_ptr = curr_PCB;
    } else {// when it is not the first process
        //store the current_ptr as the parent_ptr
        curr_PCB->parent_ptr = cur_process; //stores the current process
    }

    //initialize values in fd_array
    //initialize the stdin
    curr_PCB->fd_array[0].fd_table_ptr.read = read_terminal;
    curr_PCB->fd_array[0].fd_table_ptr.write = invalid_function;
    curr_PCB->fd_array[0].fd_table_ptr.open = open_terminal;
    curr_PCB->fd_array[0].fd_table_ptr.close = close_terminal;
    curr_PCB->fd_array[0].inode = NULL;
    curr_PCB->fd_array[0].file_position = 0;
    curr_PCB->fd_array[0].flags = ACTIVE;
    //initialize stdout
    curr_PCB->fd_array[1].fd_table_ptr.read = invalid_function;
    curr_PCB->fd_array[1].fd_table_ptr.write = write_terminal;
    curr_PCB->fd_array[1].fd_table_ptr.open = open_terminal;
    curr_PCB->fd_array[1].fd_table_ptr.close = close_terminal;
    curr_PCB->fd_array[1].inode = NULL;
    curr_PCB->fd_array[1].file_position = 0;
    curr_PCB->fd_array[1].flags = ACTIVE;
    //initialize the fd_table_entry values
    for (i = 2; i < MAX_NUM_FILES; i++){
        curr_PCB->fd_array[i].fd_table_ptr.read = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.write = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.open = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.close = invalid_function;
        curr_PCB->fd_array[i].inode = NULL;
        curr_PCB->fd_array[i].file_position = 0;
        curr_PCB->fd_array[i].flags = INACTIVE;
    }
    //store the arguments into the coressponding argument

    curr_PCB->parent_ptr->kernel_esp = tss.esp0;
    curr_PCB->parent_ptr->kernel_ss = tss.ss0;

    curr_PCB->kernel_esp =  SIZE_8MB - (KB8 * curr_PCB->process_ID) - 4;
    curr_PCB->kernel_ss = KERNEL_DS;
    strncpy(curr_PCB->args, args_buffer, MAX_arglen);
    // put ebp into temp_ebp
    asm volatile("          \n\
            movl %%ebp, %0  \n\
            " 
            : "=r" ( temp_ebp )
    );
    // put esp into temp_esp
    asm volatile("             \n\
            movl %%esp, %0;    \n\
            "
            : "=r" ( temp_esp )
    );

    curr_PCB->parent_ptr->ebp = temp_ebp;
    curr_PCB->parent_ptr->esp = temp_esp;

    /**********************************************/
    /********* SETTING UP KERNEL STACK    *********/
    /**********************************************/
    uint32_t EIP_VAL = 0;
    EIP_VAL = EIP_BUFF[3] << 24 | EIP_BUFF[2] << 16 | EIP_BUFF[1] << 8 | EIP_BUFF[0];
    tss.esp0 = curr_PCB -> kernel_esp;
    tss.ss0 = curr_PCB->kernel_ss;
    //need to load the stack

    // switch process
    cur_process = curr_PCB;
    cur_process->term = cur_terminal;

    // set terminals processs
    cur_terminal->pid = pid;
    running_terminal = cur_terminal;
    
    // 0x083FFFFC is 132 MB - 4 Bytes where the user stack should be
    

    sti();
    asm volatile(
        "movw $0x2B, %ax;"
        "movw %ax, %ds;"
    );
    asm volatile("pushl $0x0000002B" : ); //this value is USER_DS
    asm volatile("pushl $0x083FFFFC" : ); // this value is ESP_VAL, which is SIZE_128MB + SIZE_4MB - 4; - 4 because it wants to be within range and 4 because it wants it to allign properly
    asm volatile("                  \n\
                pushfl              \n\
                popl %%eax          \n\
                orl $0x200, %%eax   \n\
                pushl %%eax         \n\
    " : : : "eax" );
    // code above is done to set IF flag
    asm volatile("pushl $0x00000023" : ); // this value is USER_CS
    asm volatile("pushl %0" : : "g" (EIP_VAL));
    asm volatile("IRET");
    asm volatile("\n\
        EXECUTE_RETURN: \n\
            LEAVE       \n\
            RET         \n\
    ");


    return 0;

}


/*DESCRIPTION 
*   this function is the system call function to open
* INPUT = filename it wants to open
* OUTPUT = NONE
* SIDE EFFECTS= should open the file that it looks for
*/
/* system call function to open the file*/
int32_t open (const uint8_t* filename){
    int fd = -1; // intialization to be -1
    int i;
    dentry_t cur_dentry;
    PCB* curr_PCB = cur_process;

    if (filename == NULL  || filename < (uint8_t *)USER_SPACE_START || filename > (uint8_t *) USER_SPACE_END) return FAIL;
    
    /* select the fd table that is not in use */
    for(i = 0; i < FD_ARRAY_SIZE; i++ ){
        if(curr_PCB->fd_array[i].flags == INACTIVE){
            fd =i;
            break;
        }
    }
    
    if(fd == FAIL){
        printf("Too many files is opened!");
        return FAIL;
    }

    if( read_dentry_by_name(filename,&cur_dentry) == FAIL){
        printf("No such file exists");
        return FAIL; 
    };

    /* set up entry value in corresponding fd */
    curr_PCB->fd_array[fd].file_position = 0;
    curr_PCB->fd_array[fd].flags = ACTIVE;
    
    // if . then directory read/ directory write

    /* if it is rtc, set it to -1 */
    switch (cur_dentry.file_type) 
    {
        case rtc_TYPE:
            if (0 != rtc_open(filename)) return FAIL;
            curr_PCB->fd_array[fd].inode = -1;
            curr_PCB->fd_array[fd].fd_table_ptr.open = rtc_open;
            curr_PCB->fd_array[fd].fd_table_ptr.close = rtc_close;
            curr_PCB->fd_array[fd].fd_table_ptr.read = rtc_read;
            curr_PCB->fd_array[fd].fd_table_ptr.write = rtc_write;
            break;
        
        case terminal_TYPE:
            curr_PCB->fd_array[fd].inode = cur_dentry.inode_idx;
            curr_PCB->fd_array[fd].fd_table_ptr.open = open_terminal;
            curr_PCB->fd_array[fd].fd_table_ptr.close = close_terminal;
            curr_PCB->fd_array[fd].fd_table_ptr.read = read_terminal;
            curr_PCB->fd_array[fd].fd_table_ptr.write = write_terminal;
            
        case directory_TYPE:
            if (0 != dir_open(filename)) return FAIL;
            curr_PCB->fd_array[fd].inode = cur_dentry.inode_idx;
            curr_PCB->fd_array[fd].fd_table_ptr.open = dir_open;
            curr_PCB->fd_array[fd].fd_table_ptr.close = dir_close;
            curr_PCB->fd_array[fd].fd_table_ptr.read = dir_read;
            curr_PCB->fd_array[fd].fd_table_ptr.write = dir_write;
            break;
            
        case file_TYPE:
            if (file_open(filename) != 0) return FAIL;
            curr_PCB->fd_array[fd].inode = cur_dentry.inode_idx;
            curr_PCB->fd_array[fd].fd_table_ptr.open = file_open;
            curr_PCB->fd_array[fd].fd_table_ptr.close = file_close;
            curr_PCB->fd_array[fd].fd_table_ptr.read = file_read;
            curr_PCB->fd_array[fd].fd_table_ptr.write = file_write;
            break;
    }
    

    /* return fd for success */
    return fd;
}

/*DESCRIPTION 
*   this function is the system call function to close
* INPUT = file_directory corresponding to the one it wants to close
* OUTPUT = the close function
* SIDE EFFECTS= should close the file that it looks for
*/
/* system call function to close the file*/
int32_t close (int32_t fd){
    PCB* curr_PCB = cur_process;
    //printf("created PCB_close at :%x \n" , curr_PCB);
    /* not valid idx, try to close default, or originally closed, fail */ 
    if(fd <= 1 || fd>=FD_ARRAY_SIZE || curr_PCB->fd_array[fd].flags == INACTIVE){
        return FAIL;
    }
    /* close and call corresponding close */
    curr_PCB->fd_array[fd].flags = INACTIVE;
    return curr_PCB->fd_array[fd].fd_table_ptr.close(fd); //need to check ptr and typing, based on implementation
}

/*DESCRIPTION 
*   this function is the system call function to read
* INPUT = file_directory it wants, ptr to the buffer, and the number of bytes it wants to read
* OUTPUT = the read function
* SIDE EFFECTS= should read the file that it looks for
*/
int32_t read(int32_t fd, void* buf, int32_t nbytes){
    /* if the fd is not in range 0~7 or 1, or the flags is 0,
     * or doesn't contain read funcion, return fail
     */
    // printf("fd read : %d \n", fd);
    PCB* curr_PCB = cur_process;
    //printf("created PCB_read at :%x \n", curr_PCB);
    // printf("pointer adr read: %p \n", &buf);
    // printf("pointer points to read: %p \n", buf);
    int32_t ret;
    /* judge whether the fop_ptr is in user space */
    if( (int)buf < USER_SPACE_START || (int)buf + nbytes > USER_SPACE_END){return FAIL;}
    if (nbytes <= 0){return FAIL;}
    if (fd < 0 || fd >= FD_ARRAY_SIZE || fd == 1){return FAIL;}
    if (!curr_PCB->fd_array[fd].flags){return FAIL;}
    if (curr_PCB->fd_array[fd].fd_table_ptr.read == NULL){return FAIL;} // need to check ptr and typing due to implementation


    /* Call the corresponding read function base on the file type */
    ret = curr_PCB->fd_array[fd].fd_table_ptr.read(fd, buf,nbytes); // check this implementation
    return ret;
}

/*DESCRIPTION 
*   this function is the system call function to write
* INPUT = file_directory it wants, ptr to the buffer, and the number of bytes it wants to read
* OUTPUT = NONE
* SIDE EFFECTS= should wrie the file that it looks for by calling the function
*/
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    PCB* curr_PCB = cur_process;
    if (fd<=0 || fd >= FD_ARRAY_SIZE || !curr_PCB->fd_array[fd].flags || buf == NULL){return FAIL;}
    if(buf == NULL) {return FAIL; }
    /* judge whether the fop_ptr is in user space */
    if( (int)buf < USER_SPACE_START || (int)buf + nbytes > USER_SPACE_END){return FAIL;}
        return (curr_PCB->fd_array[fd].fd_table_ptr.write(fd,buf,nbytes));

}
/*DESCRIPTION 
*   this function is the system call function to halt
* INPUT = status
* OUTPUT = return 0 should succeed
* SIDE EFFECTS= should end the process, set the fd array values and clear the PCB
*/
int32_t halt (uint8_t status){
    cli();
    /*********************************/
    /********** CLEAN UP *************/
    /*********************************/
    int i;
    uint32_t temp_ebp;
    uint32_t temp_esp;

    running_terminal->tot_running_process--;
    PCB * curr_PCB = get_current_process();
    PCB * parent_PCB = curr_PCB->parent_ptr;

    temp_ebp = parent_PCB->ebp;
    temp_esp = parent_PCB->esp;

   // set so that process is over
    processes_array[curr_PCB->process_ID] = 0;

    // reinitialize it to basically delete it
    memset(curr_PCB->args, 0, MAX_arglen);
    curr_PCB->parent_ptr = NULL;
    curr_PCB->process_ID = -1;
    curr_PCB->kernel_esp = -1;
    curr_PCB->ebp = -1;
    curr_PCB->esp = -1;

    // maybe not needded
    for(i = 2; i < MAX_NUM_FILES; i++){
        if (curr_PCB->fd_array[i].flags == ACTIVE){
            close(i);}
        curr_PCB->fd_array[i].fd_table_ptr.close = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.open = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.read = invalid_function;
        curr_PCB->fd_array[i].fd_table_ptr.write = invalid_function;
        curr_PCB->fd_array[i].inode = NULL;
        curr_PCB->fd_array[i].file_position = 0;
        curr_PCB->fd_array[i].flags = INACTIVE;
    }

    /*********************************/
    /********** PARENT PROCESS *******/
    /*********************************/

    if(curr_PCB == parent_PCB){
        execute((uint8_t*)"shell");
    }

    cur_process = parent_PCB;
    running_terminal->pid = cur_process->process_ID;
    clear_buffer(running_terminal);


    /*********************************/
    /********** RE-SETUP PAGING ******/
    /*********************************/
    map_4mb_to_pd(SIZE_128MB, SIZE_8MB + parent_PCB->process_ID * SIZE_4MB);


    /*************************************************************/
    /********** SWITCH FILE-DESCRIPTOR ARRAY switch w parent******/
    /*************************************************************/

    /******************************************/
    /********** JUMP TO EXECUTE's RETURN ******/
    /******************************************/
    tss.esp0 = parent_PCB->kernel_esp;
    tss.ss0 = parent_PCB->kernel_ss;


    if(exception_flag == 0){
        // if not called by exception
        asm volatile("              \n\
            movl %0, %%esp          \n\
            movl %1, %%ebp          \n\
            movl %2, %%eax          \n\
            jmp EXECUTE_RETURN      \n\
        " : : "g" (temp_esp), "g" (temp_ebp), "g" ((int32_t) status) : "eax");
    } else {
        // if called by exception
        exception_flag = 0;
        asm volatile("              \n\
        movl %0, %%esp          \n\
        movl %1, %%ebp          \n\
        movl $256, %%eax          \n\
        jmp EXECUTE_RETURN      \n\
    " : : "g" (temp_esp), "g" (temp_ebp), "g" ((int32_t) status) : "eax");
    }

    sti();
    return 0;
}

/*DESCRIPTION 
*   get the arguments for the current process
* INPUT = buf, the memory location to store the arguments
          nbytes, the number of bytes read
* OUTPUT = 0, success; -1, fail
* SIDE EFFECTS=  None
*/
int32_t getargs (uint8_t* buf, int32_t nbytes) {
    if(buf == NULL) {
        return -1;
    }
    if ((int)buf < USER_SPACE_START || (int)buf + nbytes >= USER_SPACE_END)
        return -1;

    // copy over the args into the desired buffer
    PCB* pcb = cur_process;
    uint32_t len = strlen(pcb->args);
    if (len == 0 || nbytes <= len)
        return -1;
    strncpy((int8_t*)buf, pcb->args, nbytes);
    return 0;
}

/*DESCRIPTION 
*   this function is called everytime a system call for execute is being called
* INPUT = command -- parameter for what syscall it should be
* OUTPUT = returns 0;
* SIDE EFFECTS= establishes the proper paging unto VMEM and PHYMEM as well as PCB that holds the information
*/
int32_t vidmap (uint8_t** screen_start) {
    if(screen_start == NULL) {
        return -1;
    }
    if ((int)screen_start < USER_SPACE_START || (int)screen_start >= USER_SPACE_END)
        return -1;

    // set up paging for the vid memory
    map_4kb(USER_VID_ADDR, VID_ADDR);
    *screen_start = (uint8_t*)USER_VID_ADDR;
    return 0;
}

/*DESCRIPTION 
*  dummy set handler function
* INPUT = signum, the signal number
          handler_address, the address of the handler function
* OUTPUT = status of the systemcall
* SIDE EFFECTS= None
*/
int32_t set_handler (int32_t signum, void* handler_address) {
    return -1;
}

/*DESCRIPTION 
*  dummy sigreturn function
* INPUT = none
* OUTPUT = status of the systemcall
* SIDE EFFECTS= None
*/
int32_t sigreturn (void) {
    return -1;
}



/*DESCRIPTION 
*   this function gets a PCB ptr to the PCB that it is looking for
* INPUT = which process ID the PCB is in
* OUTPUT = PCB* to the PCB
* SIDE EFFECTS= NONE
*/
//gets the ptr for a specific PCB, depending on the process_ID
PCB* get_pcb_ptr (uint8_t process_ID){
    return (PCB*) (MB8 - (process_ID + 1) * KB8);
}
/*DESCRIPTION 
*   this function is a function that is created for when it wants to be a -1.
*   this needs to be done because the struct of the function is ptrs, so it needs to pass in a function
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= NONE
*/
//function used for the fd_array function since it needs to be a ptr
int32_t invalid_function(){
    return -1;
}
/*DESCRIPTION 
*   Returns the PCB of the current running process
* INPUT = NONE
* OUTPUT = PCB of the current process
* SIDE EFFECTS= NONE
*/
PCB* get_current_process() {
    return cur_process;
}
