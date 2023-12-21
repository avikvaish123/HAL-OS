#include "filesystem.h"
#include "../lib.h"

/* void init_filesystem(bootblock_t* fsImg_addr)
 * Inputs:      fsImg_addr - A pointer to the bootblock_t structure in the filesystem image
 * Return Value: void
 * Function: Initializes the filesystem by setting pointers to the boot block, inodes, and datablocks */
void
init_filesystem(bootblock_t* fsImg_addr) {
    //get filesys_img address in mp3.img, reference kernel.c to see how address is retrieved, address is not static
    bootblock = fsImg_addr;
    inode_start = (inode_t*) (bootblock+1);
    datablock_start = (datablock_t*) (inode_start + (bootblock -> numInodes));
}

/* int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)
 * Inputs:      fname - A pointer to the filename to search for in the directory entries
 *              dentry - A pointer to a dentry_t structure to store the result
 * Return Value: 0 on success, -1 on failure (non-existent file or invalid index)
 * Function: Searches for a directory entry with the specified filename and populates the dentry parameter */
int32_t
read_dentry_by_name (const uint8_t* fname, dentry_t* dentry) {
    //loop through boot block to find dentry with fname
    //fill dentry parameter with filename, filetype, and inode number of matching dentry in filesystem img, then return 0
    // scan directory entries in boot block
    uint8_t i,j;
    dentry_t* temp;

    if (strlen((const int8_t*)fname) > MAX_FILENAME_SIZE) {
        return -1;
    }

    // call read_dentry_by_index() which populates dentry parameter (file name, file type, inode num)
    for(i = 0; i < bootblock->numDentries; i++) {
        if (strncmp((const int8_t*)fname, (const int8_t*) (bootblock->bootDentries)[i].fileName, MAX_FILENAME_SIZE) == 0) {

            //simplify dentry assignments by saving dentry pointer to temp
            temp = &((bootblock->bootDentries)[i]);
            
            //iterate over each element because strncpy will stop at null characters
            for(j = 0; j < MAX_FILENAME_SIZE; j++){
                dentry->fileName[j] = temp->fileName[j];
            }

            //copy rest of the elements
            dentry->fileType = temp->fileType;
            dentry->inodeNumber = temp->inodeNumber;
            return 0;
        }
    }
    return -1;
}

/* int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry)
 * Inputs:      index - The index of the directory entry to read
 *              dentry - A pointer to a dentry_t structure to store the result
 * Return Value: 0 on success, -1 on failure (non-existent file or invalid index)
 * Function: Fills the dentry block with filename, filetype, and inode number based on the specified index */
int32_t
read_dentry_by_index (uint32_t index, dentry_t* dentry) {
//fill dentry block with filename, filetype, and inode number, then return 0
    dentry_t* temp;
    int i;

    //simplify dentry assignments by saving dentry pointer to temp
    temp = &((bootblock->bootDentries)[index]);

    //ensure current inode/index corresponds to valid directory entry
    if (bootblock->numDentries < index){
        return -1;
    }

    /* fill each of the parameters */

    //iterate over each element because strncpy will stop at null characters
    for(i = 0; i < MAX_FILENAME_SIZE; i++){
        dentry->fileName[i] = temp->fileName[i];
    }
    dentry->fileType = temp->fileType;
    dentry->inodeNumber = temp->inodeNumber;

    return 0;
}

/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs:      inode - The inode number of the file to read
 *              offset - The position from which to start reading within the file
 *              buf - A pointer to the buffer where data will be stored
 *              length - The maximum number of bytes to read
 * Return Value: The number of bytes read and placed in the buffer, 0 indicates EOF, -1 on failure
 * Function: Reads up to "length" bytes from the file with inode number "inode," starting from the position "offset," and stores the data in the buffer */
int32_t
read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
//similar to "read" system call, read up to "length" bytes, starting from position "offset" in the file with inode 
//number "inode", return the number of bytes read and places in the buffer, 0 indicates EOF
//check if inode is outside number specified in boot block
//check if data block is outside number specified in boot block
    inode_t* curr_inode = inode_start + inode;
    uint32_t curr_data_block_array_index;
    uint32_t curr_data_block_index;
    // inode range check
    // if(inode <= -1 || inode > boot_block->inode_count-1){
    if(inode > bootblock->numInodes-1 || inode < 0){ //if the target inode index doesn't exist
        printf("read_data: Inode out of range \n");
        return -1;
    }

    if(offset >= curr_inode->numBytes){                                // has the end of the file been reached by offset -> return 0 (check docs)
        return 0;
    }
    uint32_t current_byte_index; //the byte that we are currently copying
    uint8_t current_byte; //holds current byte to be written into buffer
    uint32_t num_bytes_copied = 0; //counts the number of bytes copied
    uint32_t current_byte_index_within_block; // finds the current index of a byte within a data block
    int i = 0;
    //write each individual byte into temp buffer
    for(current_byte_index = offset; current_byte_index < (offset + length); current_byte_index++){
        if(current_byte_index == curr_inode->numBytes){
            return current_byte_index - offset;
        }
        curr_data_block_array_index = current_byte_index / BLOCK_BYTES; //find the current data block index for array
        curr_data_block_index = inode_start[inode].dataBlockNumber[curr_data_block_array_index]; //find the current data block index within filesystem
        current_byte_index_within_block = current_byte_index % BLOCK_BYTES;
        current_byte = datablock_start[curr_data_block_index].dataBlockValue[current_byte_index_within_block];
        buf[i] = current_byte;
        i++;
        num_bytes_copied++;
    }
    //memcpy(buf, temp_buf, length);
    return num_bytes_copied;
}

/* int32_t directory_read(int32_t file_index, void* buff, int32_t num_bytes)
 * Inputs:      file_index - The index of the directory entry to read
 *              buf - A pointer to the buffer where the directory entry name will be stored
 *              num_bytes - The number of bytes available in the buffer
 * Return Value: The number of bytes read (string length) or -1 on failure
 * Function: Reads the name of the directory entry with the specified index and stores it in the buffer */
int32_t 
directory_read(int32_t file_index, void* buf, int32_t num_bytes) {
    if (file_index >= bootblock->numDentries) {
        return 0;
    }
    dentry_t dentry;
    if(read_dentry_by_index(file_index, &dentry) == -1){
        return -1;
    }
    
    // copy filename into buf
    int8_t* dest_ptr;
    dest_ptr = strncpy((int8_t*)buf, (int8_t*)&dentry.fileName, MAX_FILENAME_SIZE);
    if (dest_ptr == NULL){
        return -1;
    }
    int strlen_ret = strlen((int8_t*)buf);

    /* case for when filename is larger than allowed */
    
    if (strlen_ret > MAX_FILENAME_SIZE) {
        ((int8_t*)buf)[MAX_FILENAME_SIZE] = NULL_ASCII;
        return MAX_FILENAME_SIZE;
    }
    
    return strlen_ret;
}

/* int32_t directory_close(int32_t file_index)
 * Inputs:      file_index - The index of the directory entry to close
 * Return Value: 0
 * Function: Closes the directory entry with the specified index */
int32_t
directory_close(int32_t file_index){
    return 0;
}

/* int32_t directory_open(const uint8_t* fname)
 * Inputs:      fname - A pointer to the filename to open (not used for directory)
 * Return Value: 0
 * Function: Opens a directory entry with the specified filename (not used for directory entries) */
int32_t
directory_open(const uint8_t* fname){
    return 0;
}

/* int32_t directory_write(int32_t file_index, const void* buff, int32_t num_bytes)
 * Inputs:      file_index - The index of the directory entry to write (not used for directory)
 *              buf - A pointer to the data to write (not used for directory)
 *              num_bytes - The number of bytes to write (not used for directory)
 * Return Value: -1 (Read-only filesystem)
 * Function: Writes data to a directory entry with the specified index (not used for directory entries) */
int32_t
directory_write(int32_t file_index, const void* buf, int32_t num_bytes){
    return -1;
}

/* int32_t file_read(uint32_t inode_idx, int32_t offset, void* buff, int32_t num_bytes)
 * Inputs:      inode_idx - The index of the inode for the file to read
 *              offset - The offset within the file to read
 *              buf - A pointer to the buffer where the file data will be stored
 *              num_bytes - The number of bytes to read
 * Return Value: The number of bytes read or -1 on failure
 * Function: Reads data from a file specified by the inode index and file index and stores it in the buffer */
int32_t
file_read(uint32_t inode_idx, int32_t offset, void* buf, int32_t num_bytes){        
    return read_data(inode_idx, offset, buf, num_bytes);
}

/* int32_t file_write(int32_t file_index, const void* buff, int32_t num_bytes)
 * Inputs:      file_index - The index of the file to write (not used for file)
 *              buf - A pointer to the data to write (not used for file)
 *              num_bytes - The number of bytes to write (not used for file)
 * Return Value: -1 (Read-only filesystem)
 * Function: Writes data to a file with the specified index (not used for file entries) */
int32_t
file_write(int32_t file_index, const void* buf, int32_t num_bytes){
    return -1;
}

/* int32_t file_open(const uint8_t* fname)
 * Inputs:      fname - A pointer to the filename of the file to open
 * Return Value: 0
 * Function: Reads a file by filename */
int32_t
file_open(const uint8_t* fname){
    return 0;
}

/* int32_t file_close(int32_t file_index)
 * Inputs:      file_index - The index of the file to close (not used for file)
 * Return Value: Always returns 0
 * Function: Closes the file entry with the specified index (not used for file entries) */
int32_t
file_close(int32_t file_index){
    return 0;
}

/* int32_t find_dentry_by_inode_num(int32_t inode_num)
 * Inputs:      inode_num -- inode number of the dentry to find
 * Return Value: dentry index on success, -1 on failure
 * Function: Finds the dentry associated with an inode */
int32_t
find_dentry_by_inode_num(int32_t inode_num) {
    int i;

    for (i = 0; i < bootblock->numDentries; i++) {
        if ((bootblock->bootDentries[i]).inodeNumber == inode_num) {
            return i;
        }
    }

    return -1;
}
