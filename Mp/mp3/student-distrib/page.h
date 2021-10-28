#ifndef __PAGING_H__
#define __PAGING_H__

#include "types.h"

#define NUM_ENTRIES 1024
#define SIZE_4KB 4096
#define SIZE_4MB (SIZE_4KB << 10)

#define OFFSET 12
#define VID_ADDR 0xB8

typedef struct PDE_MB {
  uint32_t p : 1;
  uint32_t r_w : 1;
  uint32_t u_s : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t a : 1;
  uint32_t d : 1;
  uint32_t ps : 1;
  uint32_t g : 1;
  uint32_t avail : 3;
  uint32_t pat : 1;
  uint32_t reserved : 9;
  uint32_t base_addr : 10;
} PDE_MB_t;

typedef struct PDE_KB {
  uint32_t p : 1;
  uint32_t r_w : 1;
  uint32_t u_s : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t a : 1;
  uint32_t reserved : 1;
  uint32_t ps : 1;
  uint32_t g : 1;
  uint32_t avail : 3;
  uint32_t base_addr : 20;
} PDE_KB_t;

typedef union PDE {
  PDE_KB_t pde_4kb;
  PDE_MB_t pde_4mb;
} PDE_t;

typedef struct PTE {
  uint32_t p : 1;
  uint32_t r_w : 1;
  uint32_t u_s : 1;
  uint32_t pwt : 1;
  uint32_t pcd : 1;
  uint32_t a : 1;
  uint32_t d : 1;
  uint32_t pat : 1;
  uint32_t g : 1;
  uint32_t avail : 3;
  uint32_t base_addr : 20;
} PTE_t;

PDE_t page_directory[NUM_ENTRIES] __attribute__((aligned(SIZE_4KB)));
PTE_t page_table[NUM_ENTRIES] __attribute__((aligned(SIZE_4KB)));

extern void page_init();

#endif  // __PAGING_H__
