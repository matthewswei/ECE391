#include "syscalls.h"
#include "file_system_driver.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

/* 
 * index 0: stdin file operations table pointer (read-only)
 * index 1: stdout file operations table pointer (write-only)
 * index 2: rtc file operations table pointer
 * index 3: file file operations table pointer
 * index 4: directory file operations table pointer
 */
file_ops_t fops_table[NUM_FOPS] = {{open_fail, terminal_read, write_fail, close_fail}, {open_fail, read_fail, terminal_write, close_fail},
    {rtc_open, rtc_read, rtc_write, rtc_close}, {file_open, file_read, file_write, file_close},
    {directory_open, directory_read, directory_write, directory_close}}; /* array of possible file operations table pointers */

int32_t cur_pid = -1; /* parent ID for the initial process is -1 */

/*
 * sys_execute()
 *  Description: Executes the executable passed in as input.
 *  Inputs: command -- the executable file to execute
 *  Outputs: none
 *  Return value: 0 on success, -1 on failure
 *  Side effects: executes a user program
 */
int32_t sys_exec(const uint8_t* command){
    // puts("exec");
    int retval;
    int i;
    uint8_t * exec_data_block;
    inode_t * exec_inode;
    uint32_t program_eip;           // program start instructions
    /* Check if command is empty */ 
    if(strlen((const int8_t*)command) == 0){
        return -1;
        }
    int flag = 0;
    // int length = 0;

    // parsing arguments begins
    for(i = 0; i < strlen((const int8_t*)command); i++){
        if(command[i] == ' '){
            flag = 1;
            break;
            }
    }
    int8_t exec_name[i + 1]; // intialize string to hold exec file's name, add one to account for null character
    strncpy( (int8_t*)exec_name, (int8_t*)command, i);      // copy exec file's name from command to local string
    exec_name[i] = '\0';   

    /* GET ARGS */
    int j = 0;
    /* ignore leading spaces */
    while(command[i] == ' '){
        i++;
    }
    int args_size = strlen((const int8_t*)command) - i+1;
    int8_t args[args_size];
    /* set args to the arguments from command, after leading spaces */
    for(i = i; i < strlen((const int8_t*)command); i++){
         args[j] = command[i];
         j++;
    }
    args[args_size-1] = '\0';   // null terminate

    /* Check if file is an executable */
    dentry_t exec_dentry;
    retval = read_dentry_by_name((const uint8_t*)exec_name, &exec_dentry);
    if(retval == -1){
        return -1;          // the exec file does not exist or its name is larger than 32 characters
    }

    if(exec_dentry.file_type == EXEC_FILE_TYPE){
        exec_inode = (inode_t *)(in_memory_FS + ( (exec_dentry.inode_number + 1) * FILE_BLOCK_SIZE) );
        exec_data_block = (uint8_t *)(in_memory_FS + ((boot_block->inodes_N + 1 + exec_inode->data_block[0]) * FILE_BLOCK_SIZE) );
        /* Check for MAGIC NUMBER, IDENTIFIER " ELF" */
        if( exec_data_block[0] == EXEC_IDENT_0 && exec_data_block[1] == EXEC_IDENT_1 && exec_data_block[2] == EXEC_IDENT_2 && exec_data_block[3] == EXEC_IDENT_3){
                /* FILE IS A CORRECT EXEC */
                /* GET PROGAM EIP */
                program_eip = (uint32_t)exec_data_block[EIP_1] << 24 | (uint32_t)exec_data_block[EIP_2] << 16 | (uint32_t)exec_data_block[EIP_3] << 8 | (uint32_t)exec_data_block[EIP_4]; // cocatenate four 1 B values into one 32 bit value 
                // uint8_t* virtual_user_program = (uint8_t *)program_eip;
    

                // CREATE PCB

                cli();

                /* find the first available PCB in memory and obtain its address */
                int32_t ret = find_available_pcb();
                /* if none are available, return -1 */
                pcb_t* process;
                if (ret == -1) {
                    return -1;
                }
                else {
                    /* if one is available, create a new PCB at that address */
                    process = (pcb_t*)ret;
                }
                int32_t pcb_pid = 0;
                /* obtain the PID for this PCB based on its address */
                while (ret + ((pcb_pid+1) * EIGHT_KB) < EIGHT_MB) {
                    pcb_pid++;
                }
                process->pid = pcb_pid;  /* set the PCB's PID to the calculated value */
                process->parent_id = get_term_pid(get_cur_term());    /* set the PCB's parent ID to the previous PCB's PID */
                cur_pid = process->pid;  /* update cur_pid to be the parent for the next created PCB */
                /* open a file for stdin in index 0 of the file descriptor array */
                process->file_descriptor[STDIN].fotp = fops_table[STDIN];  /* set fops field for stdin file operations table */
                process->file_descriptor[STDIN].inode = 0;
                process->file_descriptor[STDIN].file_position = 0;
                process->file_descriptor[STDIN].flags = 1; /* set to active */
                /* open a file for stdout in index 1 of the file descriptor array */
                process->file_descriptor[STDOUT].fotp = fops_table[STDOUT];  /* set fops field for stdout file operations table */
                process->file_descriptor[STDOUT].inode = 0;
                process->file_descriptor[STDOUT].file_position = 0;
                process->file_descriptor[STDOUT].flags = 1; /* set to active */
                /* set all other files to inactive */
                process->file_descriptor[2].flags = 0;
                process->file_descriptor[3].flags = 0;
                process->file_descriptor[4].flags = 0;
                process->file_descriptor[5].flags = 0;
                process->file_descriptor[6].flags = 0;
                process->file_descriptor[7].flags = 0;
                /* set PCB fields to the values of ESP and EBP */
                register uint32_t saved_esp asm("esp");
                process->esp = saved_esp;
                process->term_esp = saved_esp;
                register uint32_t saved_ebp asm("ebp");
                process->ebp = saved_ebp;
                process->term_ebp = saved_ebp;
                process->active = 1; /* set the PCB to active */

                update_term_pid(cur_pid); // Updates terminals struct with correct process number
                //inc_term_proc(get_cur_term());

                /* Copy parsed arguments to the PCB */
                strncpy((int8_t*)process->arguments, (int8_t*)args, strlen((const int8_t*)args)+1);

                /* Set up paging */
                load_user_program(cur_pid); // set map from virtual space for user program to physical space
                flush_tlb(); // flush tlb, (clear cr3)
    
                /* Read exec data */

                /* Copy the contents of the exec file to the Virtual Memmory address stored by program_eip */
                retval = read_data(exec_dentry.inode_number, 0, (uint8_t *)VIRT_USER_LOAD, exec_inode->length); // **** copy file contents into correct offset in virtual space ****
                if(retval == -1){
                    return -1; // failed, contents of the file couldn't copy
                }
                // 8 MB 
                // prepare for context switching
                tss.esp0 =  EIGHT_MB - (cur_pid * EIGHT_KB) - PADDING; // kernel stack pointer
                tss.ss0 = KERNEL_DS;    // kernel data segment (stack segment)

                // push iret context to stack

                iret_call(program_eip);

                sti();

                return 0;
        }
        else {
            return -1;  /* file is not an executable so return -1 */
        }
    }
    // Name of File is not a regular file (labeled 2): either a directory (label 1) or rtc (label 0)
    else{
        return -1;
        }

    return 0;
}

/*
 * find_available_pcb()
 *  Description: Find the first inactive PCB and return its address in memory.
 *  Inputs: none
 *  Outputs: none
 *  Return value: the address of the PCB on success, -1 on failure
 *  Side effects: none
 */
int32_t find_available_pcb() {
    int i = 0;
    /* loop through all possible PIDs */
    for (i = 0; i < NUM_PROCESS; i++) {
        /* obtain the address of the PCB corresponding to the current PID */
        int32_t available = get_pcb_from_pid(i);
        /* check if the current PCB is active */
        if (((pcb_t*)available)->active == 0) {
            return available;   /* if it is not, return the address of this PCB */
        }
    }
    return -1;  /* all PCBs are active, so return -1 */
}

/*
 * get_pcb_from_pid()
 *  Description: Given a PID as input, this function calculates the address of
 *               the corresponding PCB.
 *  Inputs: pid -- the PID of the PCB to find
 *  Outputs: none
 *  Return value: the address of the PCB
 *  Side effects: none
 */
int32_t get_pcb_from_pid(int32_t pid) {
    /* make sure the PID is within the range of possible PIDs */
    if (pid < 0 || pid > NUM_PROCESS - 1) {return -1;}
    /* calculate the address of the PCB with this PID */
    int32_t pcb = EIGHT_MB - ((pid+1) * EIGHT_KB);    /* address = 8MB - ((PID+1) * 8KB) */
    return pcb;
}

/*
 * sys_halt()
 *  Description: Halts the program and returns to the parent process.
 *  Inputs: status -- the status of the halt
 *  Outputs: none
 *  Return value: 256 if exception, 0 otherwise
 *  Side effects: returns to the parent process and remaps memory
 */
int32_t sys_halt (uint8_t status) {
    // puts("halt");
    int i;
    int retval;
    /* if status is 255, program ended in an exception */
    if (status == EXCEPTION-1) {
        retval = EXCEPTION;   /* set return value to 256 indicating an exception */
    }
    else {
        retval = 0; /* otherwise set return value to 0 */
    }
    /* close all of the files (stdin and stdout will NOT be closed due to parameter checks) */
    for (i = 0; i < NUM_FILES; i++) {
        sys_close(i);
    }
    pcb_t* process = (pcb_t*)get_pcb_from_pid(cur_pid); /* obtain a pointer to the current PCB */
    /* if the process is active, set it to inactive */
    if (process->active != 0){
        process->active = 0;
    }
    /* check if the current process is the base process */
    if (process->parent_id == -1) {
        cur_pid = -1;
        update_term_pid(cur_pid);
        sys_exec((const uint8_t*)"shell");  /* if it is, execute shell to re-initialize everything */
    }
    else {
        /* otherwise, obtain a pointer to the PCB of the parent process */    
        pcb_t* parent = (pcb_t*)get_pcb_from_pid(process->parent_id);
        /* set the TSS values to those of the parent process */
        tss.esp0 = EIGHT_MB - ((parent->pid) * EIGHT_KB) - PADDING;
        tss.ss0 = KERNEL_DS;
        // setup previous process back into memory

        /* Unmap Current Process Frame From the  Physical Space */
        unload_user_program(cur_pid);
        /* Map Current Parent's Process Page Physical Space */
        load_user_program(parent->pid);

        /* Unmap Video Mem Page */
        unload_vidmem();

        flush_tlb();    /* flush TLB */

        parent->active = 1; /* set the parent process to active */
        cur_pid = parent->pid;  /* set the current PID to the parent's PID */
        update_term_pid(cur_pid);
        //dec_term_proc(get_cur_term());
        halt_return(process->esp, process->ebp, status);
    }
    /* set up registers for parent process */
    return retval;
}

/*
 * sys_open()
 *  Description: Initializes a file descriptor for the file and
 *               calls the file's corresponding open function.
 *  Inputs: filename -- the name of the file to open
 *  Outputs: none
 *  Return value: 0 on success, -1 on failure
 *  Side effects: updates file descriptor's fields based on type
 */
int32_t sys_open(const uint8_t* filename){
    /* parameter check */
    if (filename == NULL) {return -1;}
    dentry_t entry;
    /* obtain directory entry corresponding to the filename */
    int retval = read_dentry_by_name((const uint8_t*)filename, &entry);
    if(retval == -1) {
        return -1;  /* return -1 if read_dentry failed */
    }
    uint32_t fd = 0;
    /* obtain a pointer to the current PCB */
    pcb_t* cur_pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    int i;
    /* find first available fd for this PCB */
    for (i = 0; i < NUM_FILES; i++) {
        fd = i;
        /* if flags is 0, the current fd is not in-use so break */
        if (cur_pcb->file_descriptor[fd].flags == 0) {break;}
    }
    if (i == NUM_FILES) {return -1;}    /* all fd's are in-use, so return -1 */
    /* check if file is the RTC */
    if (entry.file_type == RTC_TYPE) {
        /* set the file ops table pointer to that of the RTC */
        cur_pcb->file_descriptor[fd].fotp = fops_table[RTC_FOTP];
        cur_pcb->file_descriptor[fd].inode = 0; /* inode field is 0 for RTC */
        cur_pcb->file_descriptor[fd].file_position = 0; /* set position to 0 */
        cur_pcb->file_descriptor[fd].flags = 1; /* set file descriptor to in-use */
    }
    /* check if file is a directory */
    else if (entry.file_type == DIR_TYPE) {
        /* set the file ops table pointer to that of a directory */
        cur_pcb->file_descriptor[fd].fotp = fops_table[DIR_FOTP];
        cur_pcb->file_descriptor[fd].inode = 0; /* inode field is 0 for directory */
        cur_pcb->file_descriptor[fd].file_position = 0; /* set position to 0 */
        cur_pcb->file_descriptor[fd].flags = 1; /* set file descriptor to in-use */
    }
    else if (entry.file_type == F_TYPE) {
        /* set the file ops table pointer to that of a file */
        cur_pcb->file_descriptor[fd].fotp = fops_table[FILE_FOTP];
        cur_pcb->file_descriptor[fd].inode = entry.inode_number; /* set inode field to inode of this file */
        cur_pcb->file_descriptor[fd].file_position = 0; /* set position to 0 */
        cur_pcb->file_descriptor[fd].flags = 1; /* set file descriptor to in-use */
    }
    else {
        return -1;
    }
    /* call the file's corresponding open function */
    int ret = cur_pcb->file_descriptor[fd].fotp.open(filename);
    if (ret == -1) {
        return -1;  /* if the open failed, return -1 */
    }
    else {
        return fd;  /* otherwise, return the fd for this file */
    }
}

/*
 * sys_close()
 *  Description: Sets file descriptor's flag to 0 and calls the file's
 *               corresponding close function.
 *  Inputs: fd -- the index of the file descriptor
 *  Outputs: none
 *  Return value: 0 on success, -1 on failure
 *  Side effects: sets file descriptor's flag field to 0
 */
int32_t sys_close(uint32_t fd){
    /* make sure fd is within possible range */
    if (fd < FIRST_NON_STD || fd > NUM_FILES-1) {return -1;}
    /* obtain a pointer to the current PCB */
    pcb_t* cur_pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    if (cur_pcb->file_descriptor[fd].flags != 1) {
        return -1;
    }
    cur_pcb->file_descriptor[fd].flags = 0; /* mark file descriptor as not in-use */
    /* call the file's corresponding close function */
    return cur_pcb->file_descriptor[fd].fotp.close(fd);
}

/*
 * sys_write()
 *  Description: Calls a file's corresponding write function.
 *  Inputs: fd -- the index of the file descriptor
 *          buf -- the buffer to write
 *          nbytes -- the number of bytes to write
 *  Outputs: none
 *  Return value: the number of bytes written
 *  Side effects: none
 */
int32_t sys_write(uint32_t fd, const void* buf, uint32_t nbytes){
    /* obtain a pointer to the current PCB */
    pcb_t* available_pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    /* parameter checks */
    if (fd < 0 || fd > NUM_FILES-1 || buf == NULL || nbytes < 0 || available_pcb->file_descriptor[fd].flags == 0){
        return -1;
    }
    /* call the file's corresponding write function */
    return available_pcb->file_descriptor[fd].fotp.write(fd, buf, nbytes);
}

/*
 * sys_read()
 *  Description: Calls a file's corresponding read function.
 *  Inputs: fd -- the index of the file descriptor
 *          buf -- the buffer to read into
 *          nbytes -- the number of bytes to read
 *  Outputs: none
 *  Return value: the number of bytes read
 *  Side effects: updates file position
 */
int32_t sys_read(uint32_t fd, void* buf, uint32_t nbytes){
    /* obtain a pointer to the current PCB */
    pcb_t* available_pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    /* parameter checks */
    if (fd < 0 || fd > NUM_FILES-1 || buf == NULL || nbytes < 0 || available_pcb->file_descriptor[fd].flags == 0){
        return -1;
    }
    /* call the file's corresponding read function */
    return available_pcb->file_descriptor[fd].fotp.read(fd, buf, nbytes);
}

/*
 * get_cur_pid()
 *  Description: Returns the current PCB's PID.
 *  Inputs: none
 *  Outputs: none
 *  Return value: the current PID
 *  Side effects: none
 */
int32_t get_cur_pid() {
    return cur_pid; /* return the current PCB's PID */
}

/*
 * sys_getargs()
 *  Description: Store the arguments of the current process into a passed in buffer.
 *  Inputs: buf -- the buffer to copy the arguments into
 *          nbytes -- number of bytes to copy into buffer
 *  Outputs: none
 *  Return value: 0 on success, -1 on failure
 *  Side effects: none
 */
int32_t sys_getargs (uint8_t* buf, int32_t nbytes){
    pcb_t* cur_pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    /* No Arguments */
    if(strlen((const int8_t*)cur_pcb->arguments) == 0){
        return -1;
    }
    /* Invalid number of bytes to be copied to buffer */
    if (nbytes <= 0) {
        return -1;
    }
    /* nbytes is greater than length of arguments, so only copy the length of arguments */
    if(nbytes > (strlen((const int8_t*)cur_pcb->arguments) + 1)){
         strncpy((int8_t*)buf, (int8_t*)cur_pcb->arguments, (strlen((const int8_t*)cur_pcb->arguments) + 1));
         return 0;
    }
    /* Otherwise, copy nbytes from current arguments into the buffer */
    strncpy((int8_t*)buf, (int8_t*)cur_pcb->arguments, nbytes);
    return 0;
}

/*
 * sys_vidmap()
 *  Description: Maps virtual video memory to physical video memory and updates screen_start.
 *  Inputs: screen_start -- pointer in the user program page to set to virtual video memory
 *  Outputs: none
 *  Return value: updated screen_start on success, -1 on failure
 *  Side effects: updates the value of *screen_start to 0x8800000
 */
int32_t sys_vidmap (uint8_t** screen_start){
 
    /* Check if within range of user-level page */
    if ((uint32_t)screen_start < VIRTUAL_USER_PROG || (uint32_t)screen_start > USER_PAGE_END) {
        return -1;
    }

    /* Dereference screen_start and set it to the virtual address of video memory */
    *screen_start = (uint8_t*)(USER_VIDMEM);
    load_vidmem(*screen_start); // set up new page for video memory
    flush_tlb();
    /* Return updated value of screen_start */
    return (int32_t)screen_start;
}

/*
 * sys_set_handler()
 *  Description: Always returns -1.
 *  Inputs: none
 *  Outputs: none
 *  Return value: always -1
 *  Side effects: none
 */
int32_t sys_set_handler (int32_t signum, void* handler_address) {
    return -1;
}

/*
 * sys_sigreturn()
 *  Description: Always returns -1.
 *  Inputs: none
 *  Outputs: none
 *  Return value: always -1
 *  Side effects: none
 */
int32_t sys_sigreturn (void) {
    return -1;
}
