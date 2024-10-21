/* file_system_driver.h - Defines for various variables and functions for the file system driver
 * vim:ts=4 noexpandtab
 */

#ifndef FILE_SYSTEM_DRIVER_H
#define FILE_SYSTEM_DRIVER_H

#include "syscalls.h"
#include "types.h"

/* Local Variables */
#define FILE_BLOCK_SIZE 4096
#define DATA_BLOCK_SIZE 4096
#define NUM_OF_D_BLOCKS  1023
#define MAX_SIZE_FNAME   32
#define MAX_NUM_DIR_ENTR  63
#define DIR_ENTRY_IDX     0

/* Struct for entries */
typedef struct dentry_t{
    uint8_t fname[32]; // 32B saved for file names
    uint32_t file_type;
    uint32_t inode_number;
    uint8_t dentry_reserved[24]; // 24B reserved in dentry
} dentry_t;

/* Struct for boot block */
typedef struct boot_block_t{
    uint32_t dir_entries;
    uint32_t inodes_N;
    uint32_t data_block_D;
    uint8_t boot_block_reserved[52]; // 52B reserved in boot block
    dentry_t dentry_in_boot[MAX_NUM_DIR_ENTR];  // 64B reserved for dentry in boot block
} boot_block_t;

/* Structure for Inode in File System Memory */
typedef struct inode_t{
    uint32_t  length;               // size of file in Bytes
    uint32_t data_block[NUM_OF_D_BLOCKS];       // data block
} inode_t;

/* Get starting address of File System */
void get_FS_addr(unsigned int input);

/* Initialize File System */
extern void file_system_init();

/* Variable holding the starting address of the File System*/
unsigned int in_memory_FS;

/* Boot Block, holds file system statistics and directory entries*/
boot_block_t* boot_block;

/* File system 'provided' functions */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* Function Prototypes for file system driver - files */
int32_t file_open(const uint8_t* filename);
int32_t file_close(uint32_t fd);
int32_t file_write(uint32_t fd, const void* buf, uint32_t nbytes);
int32_t file_read(uint32_t fd, void* buf, uint32_t nbytes);

/* Function Prototypes for file system driver - directory */
int32_t directory_open(const uint8_t* filename);
int32_t directory_close(uint32_t fd);
int32_t directory_write(uint32_t fd, const void* buf, uint32_t nbytes);
int32_t directory_read(uint32_t fd, void* buf, uint32_t nbytes);

#endif /* _FILE_SYSTEM_DRIVER_H */
