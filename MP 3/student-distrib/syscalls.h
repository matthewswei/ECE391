#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "types.h"
#include "rtc.h"
#include "keyboard.h"
#include "file_system_driver.h"
#define EXEC_FILE_TYPE  2
#define EXEC_IDENT_0    0x7F
#define EXEC_IDENT_1    0x45
#define EXEC_IDENT_2    0x4C
#define EXEC_IDENT_3    0x46
#define NUM_FOPS        5
#define EIGHT_MB        0x800000
#define EIGHT_KB        0x2000
#define NUM_PROCESS     6
#define NUM_FILES       8
#define PADDING         4
#define EXCEPTION       256
#define FIRST_NON_STD   2
#define RTC_TYPE        0
#define DIR_TYPE        1
#define F_TYPE          2
#define STDIN           0
#define STDOUT          1
#define RTC_FOTP        2
#define FILE_FOTP       3
#define DIR_FOTP        4
#define VIRT_USER_LOAD  0x08048000
#define EIP_1           27
#define EIP_2           26
#define EIP_3           25
#define EIP_4           24
#define USER_PAGE_END   0x8400000

// Function prototypes for checkpoint 3

/* Assembly linkage for system calls */
extern void system_call();

/* Flushes TLB */
extern void flush_tlb();

/* Sets up stack and iret */
extern void iret_call(uint32_t program_eip);

/* Returns to parent process to halt program */
extern void halt_return(uint32_t save_esp, uint32_t save_ebp, uint8_t status);

/* Executes the system call */
extern int32_t sys_exec (const uint8_t* command);

/* Halts the current process and returns to parent */
extern int32_t sys_halt(uint8_t status);

/* Calls corresponding read function */
extern int32_t sys_read(uint32_t fd, void* buf, uint32_t nbytes);

/* Initializes PCB file descriptor fields alls corresponding open function */
extern int32_t sys_open(const uint8_t* filename);

/* Closes PCB file descriptor fields and calls corresponding close function */
extern int32_t sys_close(uint32_t fd);

/* Calls corresponding write function */
extern int32_t sys_write(uint32_t fd, const void* buf, uint32_t nbytes);

/* Returns the PID of the current PCB */
extern int32_t get_cur_pid();

/* Copies the argments for a process into a buffer */
extern int32_t sys_getargs (uint8_t* buf, int32_t nbytes);

/* Maps virtual video memory to physical video memory */
extern int32_t sys_vidmap (uint8_t** screen_start);

/* Returns -1 */
extern int32_t sys_set_handler (int32_t signum, void* handler_address);

/* Returns -1 */
extern int32_t sys_sigreturn (void);

/* Structure for a file operations table pointer field of a file descriptor */
typedef struct file_ops{
    int32_t (*open)(const uint8_t* filename);   /* open function */
    int32_t (*read)(uint32_t fd, void* buf, uint32_t nbytes);   /* read function */
    int32_t (*write)(uint32_t fd, const void* buf, uint32_t nbytes);    /* write function */
    int32_t (*close)(uint32_t fd);  /* close function */
} file_ops_t;

/* Structure for the file descriptor of a PCB */
typedef struct file_descriptor{
    file_ops_t fotp;    /* file operations table pointer for this file */
    uint32_t inode; /* inode number for this file */
    uint32_t file_position; /* current position within the file */
    uint32_t flags; /* 1 if file is open, 0 otherwise */
} file_descriptor_t;

/* Structure for a PCB */
typedef struct pcb{
    int32_t pid;    /* process ID */
    int32_t parent_id;  /* parent's process ID */
    file_descriptor_t file_descriptor[8];   /* file descriptor table for this process */
    uint32_t esp;   /* saved stack pointer */
    uint32_t ebp;   /* saved base pointer */
    uint32_t term_esp;
    uint32_t term_ebp;
    uint32_t active;    /* 1 if PCB is active, 0 otherwise */
    uint8_t arguments[128];
} pcb_t;

/* Finds the first available PCB in memory and returns its address */
int32_t find_available_pcb();

/* Takes a PCB process ID as input and returns the address of the corresponding PCB */
int32_t get_pcb_from_pid(int32_t pid);


#endif /* _SYSCALLS_H */
