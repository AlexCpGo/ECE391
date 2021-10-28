#ifndef __FS_H__
#define __FS_H__

#include "types.h"

enum FILE_TYPES { DIR = 1, FILE };

// data blocks
#define BLOCK_SIZE 4096

typedef struct data_block {
  uint8_t data[BLOCK_SIZE];
} data_block_t;

// index node
#define NUM_DATA_BLOCKS (4096 / 4 - 1)

typedef struct inode {
  uint32_t num_bytes;
  uint32_t data_block_idxs[NUM_DATA_BLOCKS];
} inode_t;

// dir entry
#define FILENAME_LEN 32

typedef struct dentry {
  char filename[FILENAME_LEN];
  uint32_t file_type;
  uint32_t inode_idx;
  uint8_t __reserved[24];
} dentry_t;

// boot block
#define NUM_DIR_ENTRIES ((4096 - 4 - 4 - 52) / 64)

typedef struct boot_block {
  uint32_t num_dir_entries;
  uint32_t num_inodes;
  uint32_t num_data_blocks;
  uint8_t __reserved[52];
  dentry_t dir_entries[NUM_DIR_ENTRIES];
} boot_block_t;

// basic manipulation
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode,
                         uint32_t offset,
                         uint8_t* buf,
                         uint32_t length);

// init file system
void init_fs(void* fs_base_addr);

int32_t file_open(const uint8_t* filename);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_close(int32_t fd);

int32_t dir_open(const uint8_t* filename);
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t dir_close(int32_t fd);

#endif
