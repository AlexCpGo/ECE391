#define ASM     1

.globl handle_system_call, invalid, system_call_finish, system_call_table
.globl halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

handle_system_call:
    # Callee register saved
    cli
    pushl   %edi
    pushl   %esi
    pushl   %esp

    pushl   %edx
    pushl   %ecx
    pushl   %ebx

    # check if the system call number is valid (should be 1-10)
    cmpl     $1, %eax
    jl      invalid
    cmpl     $10, %eax
    jg      invalid

    sti
    call    *system_call_table(,%eax,4)
    cli
    jmp     system_call_finish

invalid:
    movl    $-1, %eax

system_call_finish:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esp
    popl %esi
    popl %edi
    sti
    
    iret

# Use a jump table to handle system call
system_call_table:
    .long   0x0
    .long   halt
    .long   execute
    .long   read
    .long   write
    .long   open
    .long   close
    .long   getargs
    .long   vidmap
    .long   set_handler
    .long   sigreturn

