#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "../types.h"

/* file name consists of up to 32 characters*/
#define FILE_NAME_SIZE      32

/* dentry has 24 bytes reserved */
#define DENTRY_RESERVED     24

/* boot block has 52 bytes reserved */
#define BOOT_RESERVED       52

/* boot block can have at most 63 dentries, one for "." and 62 for files */
#define MAX_NUM_DENTRIES    63

/* each inode can have at most 1023 data block numbers to fill up 4KB (first 4 bytes specifies number of bytes in inode) */
#define MAX_DATA_BLOCKS     1023

/* each block has 4kb */
#define BLOCK_BYTES         4096

#define NULL_ASCII          0x00
#define MAX_FILENAME_SIZE   32


/* dentry struct, each directory entry within the bootblock, reserved unused*/
typedef struct  {
    uint8_t fileName[FILE_NAME_SIZE];
    uint32_t fileType;
    uint32_t inodeNumber;
    uint8_t reserved[DENTRY_RESERVED];
} dentry_t;


/* bootblock struct, first block in the filesystem, numInodes is always 24, reserved unused*/
typedef struct {
    //number of valid files
    uint32_t numDentries;
    uint32_t numInodes;
    uint32_t numDataBlocks;
    uint8_t reserved[BOOT_RESERVED];
    dentry_t bootDentries[MAX_NUM_DENTRIES];
} bootblock_t;

/* inode struct*/
typedef struct {
    //length of file in bytes
    uint32_t numBytes;
    uint32_t dataBlockNumber[MAX_DATA_BLOCKS];
} inode_t;

typedef struct {
    uint8_t dataBlockValue[BLOCK_BYTES];
} datablock_t;

/* starting address of filesystem, pointer to bootblock */
bootblock_t* bootblock;

/* starting address of inodes, points to first inode/inode array */
inode_t* inode_start;

/* starting address of datablocks, points to first datablock/datablock array*/
datablock_t* datablock_start;

/// @brief initialize filesystem to get img starting addresses for bootblock, inodes, and datablocks
/// @param fsImg_addr 
void init_filesystem(bootblock_t* fsImg_addr);

/// @brief read denty by filename and fill input dentry
/// @param fname 
/// @param dentry 
/// @return -1 for failure (non-existent file or invalid index #), 0 for success
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);


/// @brief  read dentry by the index and fill input dentry
/// @param index 
/// @param dentry 
/// @return -1 for failure (non-existent file or invalid index #), 0 for success
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);


/// @brief read data given a certain inode at a certain offset, copy data of length bytes info buffer
/// @param inode 
/// @param offset 
/// @param buf 
/// @param length 
/// @return -1 for failure (non-existent file or invalid inode #), 0 for success
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t directory_read(int32_t file_index, void* buf, int32_t num_bytes);
int32_t directory_write(int32_t file_index, const void* buf, int32_t num_bytes);
int32_t directory_open(const uint8_t* fname);
int32_t directory_close(int32_t file_index);

int32_t file_read(uint32_t inode_idx, int32_t offset, void* buf, int32_t num_bytes);

int32_t file_write(int32_t file_index, const void* buf, int32_t num_bytes);

int32_t file_open(const uint8_t* fname);

int32_t file_close(int32_t file_index);

int32_t find_dentry_by_inode_num(int32_t inode_num);

#endif /* _FILESYSTEM_H */
