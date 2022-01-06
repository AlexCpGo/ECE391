#include "scheduler.h"
#include "terminal.h"
#include "page.h"
#include "system_execute_c.h"


/* void schedule()
 * input: none
 * output: none
 * function: schedules the current process to be running in the background and updates the esp, ebp and the cur_process pcb
 */
void schedule() {
    cli();
    //first terminal
    if (running_terminal == NULL) {
        running_terminal = cur_terminal;
    }

    // find next terminal
    int i;
    int32_t term_id = running_terminal->id;
    for (i = 0; i < NUM_TERMINALS; i++) {
        term_id++;
        term_id %= NUM_TERMINALS;
        if (terminals[term_id].pid != -1)
            break;
    }

    if (term_id == running_terminal->id)
        return;

    terminal_t* next_term = &terminals[term_id];

    // remap vid
    if (next_term == cur_terminal) // term being displayed
        {
            map_4kb(USER_VID_ADDR, VID_ADDR);
            // change_vidmem(VID_ADDR);
        }
    else // term not displayed
        {
            map_4kb(USER_VID_ADDR, (uint32_t)next_term->vid_mem);
            // change_vidmem(next_term->vid_mem);
        }

    // context switch
    PCB* pcb_curr = get_pcb_ptr(running_terminal->pid);
    PCB* pcb_next = get_pcb_ptr(next_term->pid);
    // remap
    map_4mb_to_pd(SIZE_128MB, SIZE_8MB + pcb_next->process_ID * SIZE_4MB);
    // restore tss
    tss.ss0 = pcb_next->kernel_ss;
    tss.esp0 = pcb_next->kernel_esp;

    // change terminal
    running_terminal = next_term;
    cur_process = pcb_next;

    // save ebp and esp
    uint32_t ebp, esp;
    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        :"=r"(esp), "=r"(ebp)
        :
    );
    pcb_curr->ebp = ebp;
    pcb_curr->esp = esp;
    // restore ebp and esp
    ebp = pcb_next->ebp;
    esp = pcb_next->esp;
    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        :"r"(esp), "r"(ebp)
    );
    sti();
}
