#include "page.h"
/*
 * init page table:
 *	description: initialize the page table, and page directory
 *	input: None
 *	output: None
 */
void init_page_table() {
    int i;
    for (i = 0; i < NUM_ENTRIES; i++) {
        // init page table
        page_table[i].p = 0;
        page_table[i].r_w = 1;
        page_table[i].u_s = 0;
        page_table[i].pwt = 0;
        page_table[i].pcd = 0;
        page_table[i].a = 0;
        page_table[i].d = 0;
        page_table[i].pat = 0;
        page_table[i].g = 0;
        page_table[i].avail = 0;
        page_table[i].base_addr = i;

        // init page dirs
        page_directory[i].pde_4kb.p = 0;
        page_directory[i].pde_4kb.r_w = 1;
        page_directory[i].pde_4kb.u_s = 0;
        page_directory[i].pde_4kb.pwt = 0;
        page_directory[i].pde_4kb.pcd = 0;
        page_directory[i].pde_4kb.a = 0;
        page_directory[i].pde_4kb.reserved = 0;
        page_directory[i].pde_4kb.ps = 0;
        page_directory[i].pde_4kb.g = 0;
        page_directory[i].pde_4kb.avail = 0;
        page_directory[i].pde_4kb.base_addr = i;
    }
    // first dir
    page_directory[0].pde_4kb.p = 1;
    page_directory[0].pde_4kb.base_addr = (uint32_t)page_table >> OFFSET;

    // second dir
    page_directory[1].pde_4mb.p = 1;
    page_directory[1].pde_4mb.ps = 1;
    page_directory[1].pde_4mb.base_addr = 0x1;

    // video mem
    page_table[VID_ADDR].p = 1;
}

/*
 * enable_paging:
 *	description: enable the page, let the page store the address
 *	input: None
 *	output: None
 */
void page_enable() {
    asm volatile (
        // cr3, base addr
        "movl $page_directory, %eax;"
        "andl $0xfffffc00, %eax;"
        "movl %eax, %cr3;"

        // cr4, enable 4kb and 4mb
        "movl %cr4, %eax;"
        "orl $0x10, %eax;"
        "movl %eax, %cr4;"

        // cr0, enable paging
        "movl %cr0, %eax;"
        "orl $0x80000001, %eax;"
        "movl %eax, %cr0;"
    );
}
/*
 * init_paging:
 *	description: initialize the paging process, followed by initialize page table, and then enable paging work.
 *	input: None
 *	output: None
 */
void page_init() {
    init_page_table();
    page_enable();
}
