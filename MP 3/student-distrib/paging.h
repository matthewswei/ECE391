/* x86_desc.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

#ifndef _PAGING_H
#define _PAGING_H

#define VIDEO_MEM_START     0xB8000
#define VIDEO_MEM_END       0xC0000
#define KERNEL_MEM_START      0x400000
#define PAGE_4K_SIZE        4096
#define VIRTUAL_USER_PROG   0x08000000
#define VIRTUAL_USER_PROG   0x08000000
#define PHYS_USER_PROG_STR    0x800000
#define SIZE_4MB             0x400000
#define USER_VIDMEM          0x8800000
#define TEN_LSB_MASK         0x3FF
#define TERMINAL_START       0xB9000
#define VIDMEM_SIZE          0x1000

#include "x86_desc.h"

/* A Page Directory Entry (4KB Page Table)*/
typedef struct p_directory_entry_4k_pt {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3;
        uint32_t pt_base_address : 20;
} p_directory_entry_4k_pt;

/* A Page Table Entry (4KB Page )*/
typedef struct p_table_entry_4k_p {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t page_table_attribute_index : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3;
        uint32_t page_base_address : 20;
}p_table_entry_4k_p;

/* A Page-Directory Entry (4-MByte Page)*/
typedef struct  p_directory_entry_4m_p {
        uint32_t present : 1;
        uint32_t read_write : 1;
        uint32_t user_supervisor : 1;
        uint32_t pwt : 1;
        uint32_t pcd : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t page_size : 1;
        uint32_t global_page : 1;
        uint32_t avail : 3;
        uint32_t page_table_attribute_index : 1;
        uint32_t reserved : 9;
        uint32_t page_base_address : 10;
}p_directory_entry_4m_p;

// Types of entries the Page Directory can take on (Union size is 32 bits (4 Bytes), only one field active at a time)
typedef union directory_entry{
    p_directory_entry_4k_pt   _4k_pt;             // 4 KB Page Table
    p_directory_entry_4m_p    _4m_p;              // 4 MB page

}directory_entry;
/* The page directory (declared in x86_desc.S */
extern union directory_entry page_directory[P_DIREC_SIZE];

/* The page table (declared in x86_desc.S) */
extern struct p_table_entry_4k_p page_table[P_TABLE_SIZE];

/* Second page table, for user video memory page (declared in x86_desc.S) */
extern struct p_table_entry_4k_p page_table_new[P_TABLE_SIZE];

/* Set Up Paging for Transferring Virtual Memory to Physical Memory */
extern void page_init();

/* Initialize the OS's respective control registers to allow Paging */
extern void set_control_registers(unsigned int* page_dir);

/* Set up Paging for User Program in Physical Memory */ 
extern void load_user_program(uint32_t process_number);

/* Unmap Current proccess from physical memory */
extern void unload_user_program(uint32_t process_number);

/* Map virtual video memory to physical video memory */
extern void load_vidmem (uint8_t* screen_start);

/* Unmap virtual video memory from physical video memory */
extern void unload_vidmem();

/* Copies and saves video memory from one terminal's spot in memory to another */
extern void copy_to_vidmem(unsigned int old_terminal, unsigned int new_terminal);
 
#endif /* _PAGING_H */
