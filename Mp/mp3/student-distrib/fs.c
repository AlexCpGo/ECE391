#include "lib.h"
#include "fs.h"

// boot block addr
boot_block_t* boot_block;

// inode arr
inode_t* inodes;

// data block arr
data_block_t* data_blocks;

// no opened dir
uint32_t cur_dir = -1; 

/* init_fs
 *  
 *  DESCRIPTION: this function initializes the fielsystem
 *  INPUT: fs_base_addr
 *  OUTPUT: NONE
 *  SIDE EFFECTS: set the starting address for boot_block, inodes, and data_blocks.
 */
void init_fs(void* fs_base_addr) {
    boot_block = (boot_block_t*)fs_base_addr;
    inodes = (inode_t*)&boot_block[1];
    data_blocks = (data_block_t*)&inodes[boot_block->num_inodes];
}

/* read_dentry_by_name
 *   
 *  DESCRIPTION: this function is to read the directory entry by name
 *  INPUT: fname, dentry
 *  OUTPUT: 0/-1
 *  SIDE EFFECTS: copy entry data to directory entry
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    if (fname == NULL || dentry == NULL) // invalid arguments
        return -1;

    int i;
    char fname_buf[FILENAME_LEN+1] = {0};
    strncpy(fname_buf, (int8_t*)fname, FILENAME_LEN);
    for (i = 0; i < boot_block->num_dir_entries; i++) {
        dentry_t* cur = boot_block->dir_entries + i;
        if (strncmp(cur->filename, fname_buf, FILENAME_LEN) == 0) {
            // copy entry data on success
            *dentry = *cur;
            return 0;
        }
    }
    return -1;
}

/* read_dentry_by_index
 *
 *  DESCRIPTION: this function is to read the directory entry by index
 *  INPUT: index, dentry
 *  OUTPUT: 0/-1
 *  SIDE EFFECTS: copy entry data to directory entry
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if (index < boot_block->num_dir_entries) {
        *dentry = boot_block->dir_entries[index];
        return 0;
    }
    return -1;
}

/* read_data
 *
 *  DESCRIPTION: this function is to read the data from the data_blocks
 *  INPUT: inode_idx, offset, buf, length
 *  OUTPUT: 0/-1/len_buf
 *  SIDE EFFECTS: copy block data to a buffer, and return the length of the buffer
 */
int32_t read_data(uint32_t inode_idx,
        uint32_t offset,
        uint8_t* buf,
        uint32_t length) {
    
    if (buf == NULL)
        return -1;
    if (inode_idx >= boot_block->num_inodes)
        return -1;
    
    inode_t* inode = inodes + inode_idx;

    if (offset >= inode->num_bytes) // offset too large
        return 0;

    if (length + offset > inode->num_bytes)
        length = inode->num_bytes - offset;
    uint32_t len_buf = length;
    
    uint32_t block_num = offset / BLOCK_SIZE;
    uint32_t block_offset = offset % BLOCK_SIZE;
    while (length > 0) {
        uint32_t block_id = inode->data_block_idxs[block_num];
        uint32_t read_len = BLOCK_SIZE - block_offset;
        if (read_len > length) // don't read too much data
            read_len = length;
        
        length -= read_len;
        while (read_len-- > 0) {
            *buf = data_blocks[block_id].data[block_offset];
            buf++;
            block_offset++;
        }

        block_num++;
        block_offset = 0;
    }
    return len_buf;
}

/* file_open
 *  
 *  DESCRIPTION: this function is to check wether there is this file
 *  INPUT: filename
 *  OUTPUT: 0/-1
 *  SIDE EFFECTS: check wether there is this file, and return the check result
 */
int32_t file_open(const uint8_t* filename) {
    dentry_t dir;
    if (read_dentry_by_name(filename, &dir) != 0)
        return -1;
    if (dir.file_type != FILE)
        return -1;
    return 0;
}

/* file_read
 *
 *  DESCRIPTION: this function is to read the file
 *  INPUT: fd, buf, nbytes
 *  OUTPUT: 0
 *  SIDE EFFECTS: no thread using it
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    return 0;
}


/* file_write
 *
 *  DESCRIPTION: this function is to write the file
 *  INPUT: fd, buf, nbytes
 *  OUTPUT: -1
 *  SIDE EFFECTS: no thread using it
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/* file_close
 *
 *  DESCRIPTION: this function is to close the file
 *  INPUT: fd
 *  OUTPUT: 0
 *  SIDE EFFECTS: no thread using it
 */
int32_t file_close(int32_t fd) {
    return 0;
}


/* dir_open
 *
 *  DESCRIPTION: this function is to open the directory
 *  INPUT: pointer to the filename
 *  OUTPUT: 0
 *  SIDE EFFECTS: open a directory, start from 0
 */
int32_t dir_open(const uint8_t* filename) {
    cur_dir = 0; // open dir, start from 0
    return 0;
}


/* dir_read
 *  
 *  DESCRIPTION: this function is to read the directory
 *  INPUT: fd, buf, nbytes
 *  OUTPUT: 0/-1
 *  SIDE EFFECTS: check wether we can read the directory, if we can, return the length of the storage buffer
*/
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL || cur_dir == -1)
        return -1;
    if (cur_dir >= boot_block->num_dir_entries) { // finished reading all dirs
        return 0;
    }

    uint32_t len = FILENAME_LEN < nbytes ? FILENAME_LEN : nbytes;
    char* c_buf = buf;
    strncpy(c_buf, boot_block->dir_entries[cur_dir].filename, len);
    c_buf[len] = '\0';
    cur_dir++; // next call should read the next dir
    return len;
}


/* dir_write
 *  DESCRIPTION: this function is to write the directory
 *  INPUT: fd, buf, nbytes
 *  OUTPUT: -1
 *  SIDE EFFECTS: no thread using it
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/* dir_close
 * 
 *  DESCRIPTION: this function is to close the directory
 *  INPUT: fd
 *  OUTPUT: -1 for closing directory
 *  SIDE EFFECTS: no thread using it
 */
int32_t dir_close(int32_t fd) {
    cur_dir = -1; // close dir
    return 0;
}
