/* file_system_driver.c - Functions to intialize, handle, and operate the file system
 * vim:ts=4 noexpandtab
 */

#include "file_system_driver.h"
#include "lib.h"


/* 
 * get_FS_addr()
 *   DESCRIPTION: sets a global variable to the starting address for file system
 *   INPUTS: input from kernel.c
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: initializes variable to starting address of file system
 */
void get_FS_addr(unsigned int input) {
    in_memory_FS = input;
}

/* 
 * file_system_init()
 *   DESCRIPTION: initialize boot block
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: creates a pointer to the boot block struct
 */
void file_system_init() {
    boot_block = (boot_block_t *)in_memory_FS;
    return;
}

/* THREE ROUTINES PROVIDED BY THE FILE SYSTEM (we still have to write) BEGIN */

/* 
 * read_dentry_by_name()
 *   DESCRIPTION: copy data from boot block to dentry based on file names
 *   INPUTS: fname -> current file index; dentry -> pointer to dentry struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    int fname_length, i;
    fname_length = strlen((int8_t *)fname);
    /* check if file name is longer than 32 bytes in size */
    if (fname_length > MAX_SIZE_FNAME) {
        return -1;          // invalid file name entry over allowed size
    }
    for (i = 0; i < boot_block->dir_entries; i++) {
        if (strncmp((int8_t*) fname, (int8_t*)boot_block->dentry_in_boot[i].fname, MAX_SIZE_FNAME) == 0) {
            strncpy((int8_t*)dentry->fname, (int8_t*)boot_block->dentry_in_boot[i].fname, MAX_SIZE_FNAME);  // copy file neame to passed in directory entry
            dentry->file_type = boot_block->dentry_in_boot[i].file_type;                                    // copy file type to passed in directory entry
            dentry->inode_number = boot_block->dentry_in_boot[i].inode_number;                              // copy inode # to passed in directory entry
            return 0;           
        }
    }
    return -1; //file name does not exist in directory
}   

/* 
 * read_dentry_by_index()
 *   DESCRIPTION: copy data from boot block to dentry based on index
 *   INPUTS: index -> current file index; dentry -> pointer to dentry struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    int i;
    if (index < MAX_NUM_DIR_ENTR || index >= 0) { // boot block size is 64 but we are 0 indexed, so we actually check if index is less than 64-1
            // copy file name to passed in directory entry
            for (i = 0; i < MAX_SIZE_FNAME; i++) {
                dentry->fname[i] = boot_block->dentry_in_boot[index].fname[i];
            }
            //strncpy((int8_t*)dentry->fname, (int8_t*)boot_block->dentry_in_boot[i].fname, 32);
            dentry->file_type = boot_block->dentry_in_boot[index].file_type;            // copy file type to passed in directory entry
            dentry->inode_number = boot_block->dentry_in_boot[index].inode_number;      // copy inode # to passed in directory entry
            return 0;
    }
    return -1; // index is out of range 
}

/* 
 * read_data()
 *   DESCRIPTION: transfer data from file to buffer
 *   INPUTS: inode -> current file inode we are looking at
 *           offset -> used to find starting address
 *           buf -> buffer which we are copying data into
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    int i;                          // loop index
    inode_t* current_inode;         // current file's inode being examined
    int file_length;                // number of bytes the file recquires
    int data_block_idx;             // index of data block within inode struct
    uint8_t* data_block;            // head address of data block
    int copied;                     // number of bytes copied to buffer
    int idx;                        // buffer loop index
    uint32_t length_ = length;

    /*  Check if inode is inbounds*/ 
    if (inode >= boot_block->inodes_N || inode < 0) {
        return -1;              // inode not in range
    }
    /* Get inode starting address in memory */
    current_inode = (inode_t *)(in_memory_FS + (inode + 1) * FILE_BLOCK_SIZE);

    /* Get File Length in Bytes */
    file_length = current_inode->length;
    /* Check if length to copy to buffer is larger than file length */
    if (length_ > file_length) {
       length_ = file_length;
    }

    /* Get index of Data Block */
    data_block_idx = offset / DATA_BLOCK_SIZE;

    /* Grab address of Data Block in File System */
    //data_block  = current_inode->data_block[data_block_idx]; 

    data_block = (uint8_t *)(in_memory_FS + ((boot_block->inodes_N + 1+ current_inode->data_block[data_block_idx]) * FILE_BLOCK_SIZE) );

    /* Get location of starting address to read from in Data Block */
    unsigned int start = offset % DATA_BLOCK_SIZE;

    /* Transfer file data to the buffer */
    copied = 0;
    idx = 0;
    while (data_block_idx < NUM_OF_D_BLOCKS) {
        for (i = start; i < DATA_BLOCK_SIZE; i++) {
            if (copied < length_) {
                buf[idx] = data_block[i]; 
                copied++;
                idx++;
            } else {
                break;
            }
        }
        /* Copied all elements, from one data block */
        if (copied == length_) {
            return copied; // return the number of bytes successfully read and placed into the buffer
        }
        /* Not all Bytes were added to buffer, some bytes exist outside of the data block examined */
        data_block_idx ++; // go to next data block
        start = 0;         // begin at top of the data block
        //data_block  = (unsigned int *)current_inode->data_block[data_block_idx];   
        data_block = (uint8_t *)(in_memory_FS + ((boot_block->inodes_N + 1+ current_inode->data_block[data_block_idx]) * FILE_BLOCK_SIZE) ); // go to next data block's address in memory
    }
    return length_;
}

/* THREE ROUTINES PROVIDED BY THE FILE SYSTEM (we still have to write) END */


/* FILE SYSTEM DRIVER - FILES BEGIN */

/* 
 * file_open()
 *   DESCRIPTION: initialize all temporary structures
 *   INPUTS: filename -> current file we are looking at
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t file_open(const uint8_t* filename) {
    dentry_t temp_dentry;
    return read_dentry_by_name (filename, &temp_dentry);
}

/* 
 * file_close()
 *   DESCRIPTION: undo what we did in open function
 *   INPUTS: fd -> value for current file
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t file_close(uint32_t fd) {
    /* COMPLETE */
    // all initial structures are initialized in init function, so do nothing here
    return 0;
}

/* 
 * file_write()
 *   DESCRIPTION: do nothing
 *   INPUTS: fd -> value for current file; buf -> buffer; nbytes -> data to copy over
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t file_write(uint32_t fd, const void* buf, uint32_t nbytes) {
    return -1;              
}

/* 
 * file_read()
 *   DESCRIPTION: reads count bytes of data from file into buf
 *   INPUTS: fd -> value for current file; buf -> buffer; nbytes -> data to copy over
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on failed conditions; 0 on success
 *   SIDE EFFECTS: none
 */
/*fd index within directory entries*/ 
int32_t file_read(uint32_t fd, void* buf, uint32_t nbytes) {
    if(fd > 7 || fd < 2){ // out of bounds check
        return -1;
    }

    if(nbytes < 0){ // out of bounds check
        return -1; // negative bytes to copy
    }
    
    //uint8_t* buff = (uint8_t*)buf;
    pcb_t* cur_pcb = (pcb_t *)get_pcb_from_pid(get_cur_pid());
    file_descriptor_t* curr_fd_entry = &(cur_pcb->file_descriptor[fd]);

    inode_t* current_inode_pos = (inode_t *)(in_memory_FS + (curr_fd_entry->inode + 1) * FILE_BLOCK_SIZE);
    if(curr_fd_entry->file_position >= current_inode_pos->length){
        return 0;
    }
    /* Get respective inode related to file */
    //dentry_t curr_direc_entry = boot_block->dentry_in_boot[fd];
    //uint32_t curr_inode  = curr_direc_entry.inode_number;
    uint32_t out = 0;
    /* Check if nbytes is greater than the remaining characters in the file */
    if (nbytes > (current_inode_pos->length - curr_fd_entry->file_position)) {
        /* If so, only read the remaining characters */
        out = read_data(curr_fd_entry->inode, curr_fd_entry->file_position, (uint8_t *)buf, current_inode_pos->length - curr_fd_entry->file_position);
    }
    else {
        /* Otherwise, read nbytes characters */
        out = read_data(curr_fd_entry->inode, curr_fd_entry->file_position, (uint8_t *)buf, nbytes);
    }
    curr_fd_entry->file_position += out; // increment offset to begin at ending address
    
    //buf = (void*)buff;
   
    /* Examine if entire file's contents were read */
    // if(curr_fd_entry->file_position > current_inode_pos->length){
    //     out = 0;
    // }
    return out;
}

/* FILE SYSTEM DRIVER - FILES END */


/* FILE SYSTEM DRIVER - DIRECTORY BEGIN */

/* 
 * directory_open()
 *   DESCRIPTION: opens a directory file
 *   INPUTS: filename -> name of directory file we are looking at
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t directory_open(const uint8_t* filename) {
    if(strncmp((int8_t*) filename, (int8_t*)boot_block->dentry_in_boot[DIR_ENTRY_IDX].fname, MAX_SIZE_FNAME) == 0){
        return 0;
    }
    return -1;
}


/* 
 * directory_close()
 *   DESCRIPTION: do nothing
 *   INPUTS: fd -> value for current file
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t directory_close(uint32_t fd) {
    return 0;
}


/* 
 * directory_write()
 *   DESCRIPTION: do nothing
 *   INPUTS: fd -> value for current file; buf -> buffer; nbytes -> data to copy over
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t directory_write(uint32_t fd, const void* buf, uint32_t nbytes) {
    return -1;
}


// /* 
//  * directory_read()
//  *   DESCRIPTION:  read files filename by filename, including “.”
//  *   INPUTS: fd -> value for current file; buf -> buffer; nbytes -> data to copy over
//  *   OUTPUTS: none
//  *   RETURN VALUE: -1 on failed conditions; 0 on success
//  *   SIDE EFFECTS: none
//  */
int32_t directory_read(uint32_t fd, void * buf, uint32_t nbytes) {
    dentry_t dir_entry;
    int i;
    int val;
    int read = MAX_SIZE_FNAME;
    uint8_t* buff = (uint8_t*)buf;
    /* Out of Bounds Check */ 
    if(fd > 7 || fd < 0){ // out of bounds check
        return -1;
    }
    if (nbytes < 0 || nbytes > MAX_SIZE_FNAME) {
        return -1;       // see if number of bytes to copy is less than 32 (size of a file name)
        }
     if (nbytes == 0) {
        return 0;
     }
    
    pcb_t* cur_pcb = (pcb_t *)get_pcb_from_pid(get_cur_pid());
    file_descriptor_t* curr_fd_entry = &(cur_pcb->file_descriptor[fd]);

    val = read_dentry_by_index(curr_fd_entry->file_position, &dir_entry);        // get directory entry for respective file's index within directory
    if (val == -1) {
        return -1;
    }
    // Copy filename from directory entries to buffer
    for (i = 0; i < MAX_SIZE_FNAME; i++) {
        buff[i] = dir_entry.fname[i];
    }
    buf = (void*)buff;
    curr_fd_entry->file_position ++; // increment file position to the next file name

    /* Examine if all files were read within directory */
    if(curr_fd_entry->file_position == boot_block->dir_entries + 1){
        read = 0;
    }
    return read; // return the number of bytes copied to buffer, should be equal to nbytes
}
/* FILE SYSTEM DRIVER - DIRECTORY END */
