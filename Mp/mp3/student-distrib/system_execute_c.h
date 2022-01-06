#include "fs.h"
#include "lib.h"
#include "page.h"
#include "terminal.h"
#include "types.h"
#include "x86_desc.h"

#define program_image_addr 0x08048000
#define MAX_arglen 128
#define CHECK_EXECUTABLE_BUFF_LEN 28
#define SUCCESS 0
#define FAIL -1

#define MAX_NUM_FILES 8
#define MAX_NUM_PROCESS 6
#define MB8 0x00800000
#define KB8 0x2000
#define MB128_MB4 0x7FFFFFC
#define ACTIVE 1
#define INACTIVE 0

#define FD_ARRAY_SIZE 8
#define USER_SPACE_START 0x8000000
#define USER_SPACE_END 0x8400000
#define USER_VID_ADDR 0x8c00000

#define rtc_TYPE 0
#define terminal_TYPE 1
#define file_TYPE 2
#define directory_TYPE 3

#define MAGIC_NUMBER 4



// extern uint8_t ls_FLAG;
// fo_table struct
typedef struct {  // follows appendix, essentially becomes a function ptrs to
                  // read, write, open and close
  int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
  int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
  int32_t (*open)(const uint8_t* filename);
  int32_t (*close)(int32_t fd);
} fo_table;

// file_descriptor_table struct
// holds information for files being opened
typedef struct {  // follows documentation
  fo_table fd_table_ptr;
  int32_t inode;
  int32_t file_position;
  int32_t flags;
  // uint32_t status; // is it being used or not
} fd_table_entry;

// PCB struct
// holds information that the PCB needs to hold
typedef struct PCB {  // supposed to store information ABOUT a process
  fd_table_entry fd_array[MAX_NUM_FILES];  // 8 because it can run 8
  uint32_t ebp;
  uint32_t esp;
  uint8_t process_ID;
  uint32_t kernel_esp;
  uint32_t kernel_ss;
  struct PCB* parent_ptr;  // ptr used to keep track of the parent, which can
                           // access everything the parent has
  int8_t args[MAX_arglen];

  terminal_t* term; // the terminal this process runs on

  // open files list -- list of files opened for a process
  // what is being typed on the terminal?
} PCB;

extern PCB* cur_process;

/*----------------------------Sytem Call Main
 * Function---------------------------------*/
/* halt system call*/
extern int32_t halt(uint8_t status);
/* execute system call*/
extern int32_t execute(const uint8_t* command);
/* read system call*/
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
/* write system call*/
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
/* open system call*/
extern int32_t open(const uint8_t* filename);
/* close system call*/
extern int32_t close(int32_t fd);
/* getargs system call*/
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
/* vidmap system call*/
extern int32_t vidmap(uint8_t** screen_start);
/* set_handler system call*/
extern int32_t set_handler(int32_t signum, void* handler_address);
/* sigreturn system call*/
extern int32_t sigreturn(void);

/*----------------------------Sytem Call Helper
 * Function---------------------------------*/
/* invalid_function for fd_array*/
extern int32_t invalid_function();
/* helper function to get the ptr to memory for the PCB*/
extern PCB* get_pcb_ptr(uint8_t process_ID);

/*  Returns the PCB of the current running process */
PCB* get_current_process();

// fd_entry_t* get_fd_array();

// void set_fd(fd_entry_t* fd_array_in)

// 0x0000002B
// 0x083FFFFC
// 0x00000023
