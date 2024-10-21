/* paging.c - Functions to initialize paging
 * vim:ts=4 noexpandtab
 */

#include "paging.h"
#include "lib.h"

/* 
 * page_init
 *   DESCRIPTION: Map virtual memory for the video memory, which is a 4KB
 *                paged stored at VIDEO_MEM_START,  and the 
 *                Kernel, which spans from 4 MB to 8MB to physical memory.
 *                Correctly set the control registers of the OS to allow 
 *                paging.   
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Maps Virtual Memory to Physical Memory
 */
void page_init(){
    int i; // loop index

    /* Initialize the Page Directory Table to Empty */
    for(i = 0; i < P_DIREC_SIZE; i++) {
        page_directory[i]._4k_pt.present = 0;
        page_directory[i]._4k_pt.read_write = 0;   
        page_directory[i]._4k_pt.user_supervisor = 0;
        page_directory[i]._4k_pt.pwt = 0;
        page_directory[i]._4k_pt.pcd = 0;
        page_directory[i]._4k_pt.accessed = 0;
        page_directory[i]._4k_pt.reserved = 0;
        page_directory[i]._4k_pt.page_size = 0;
        page_directory[i]._4k_pt.global_page = 0;
        page_directory[i]._4k_pt.avail = 0;
        page_directory[i]._4k_pt.pt_base_address = 0;
    }
    /* Initialize the Page Table to Empty */
    for(i = 0; i < P_TABLE_SIZE; i++) {
        page_table[i].present = 0;
        page_table[i].read_write = 0;          
        page_table[i].user_supervisor = 0;
        page_table[i].pwt = 0;
        page_table[i].pcd = 0;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_table_attribute_index = 0;
        page_table[i].global_page = 0;
        page_table[i].avail = 0;
        page_table[i].page_base_address = i;    // since bottom 12 bits are zeros, each page is separated by 4K           
    }

    /* Initialize the second page table to empty */
    for(i = 0; i < P_TABLE_SIZE; i++) {
        page_table_new[i].present = 0;
        page_table_new[i].read_write = 0;          
        page_table_new[i].user_supervisor = 0;
        page_table_new[i].pwt = 0;
        page_table_new[i].pcd = 0;
        page_table_new[i].accessed = 0;
        page_table_new[i].dirty = 0;
        page_table_new[i].page_table_attribute_index = 0;
        page_table_new[i].global_page = 0;
        page_table_new[i].avail = 0;
        page_table_new[i].page_base_address = i;    // since bottom 12 bits are zeros, each page is separated by 4K           
    } 

    /* Transfer Video Memory addresses*/

    /* Starting index of Video Memory*/
    int video_start_idx = (uint32_t)VIDEO_MEM_START >> 12; // index should be at 184 out of 1024
    page_table[video_start_idx].present = 1;              //set present bit
    page_table[video_start_idx].read_write = 1;           // set read write bit
    page_table[video_start_idx].user_supervisor = 1;      // set supervisor bit   
    page_table[video_start_idx].pwt = 0;
    page_table[video_start_idx].pcd  = 0;
    page_table[video_start_idx].accessed  = 0;
    page_table[video_start_idx].dirty = 0;
    page_table[video_start_idx].page_table_attribute_index = 0;
    page_table[video_start_idx].global_page = 0;
    page_table[video_start_idx].avail = 0;
    
    /* Starting index of Video Memory*/
    video_start_idx = (uint32_t)0xB9000 >> 12; // index should be at 184 out of 1024
    page_table[video_start_idx].present = 1;              //set present bit
    page_table[video_start_idx].read_write = 1;           // set read write bit
    page_table[video_start_idx].user_supervisor = 1;      // set supervisor bit   
    page_table[video_start_idx].pwt = 0;
    page_table[video_start_idx].pcd  = 0;
    page_table[video_start_idx].accessed  = 0;
    page_table[video_start_idx].dirty = 0;
    page_table[video_start_idx].page_table_attribute_index = 0;
    page_table[video_start_idx].global_page = 0;
    page_table[video_start_idx].avail = 0;

    /* Starting index of Video Memory*/
    video_start_idx = (uint32_t)0xBA000 >> 12; // index should be at 184 out of 1024
    page_table[video_start_idx].present = 1;              //set present bit
    page_table[video_start_idx].read_write = 1;           // set read write bit
    page_table[video_start_idx].user_supervisor = 1;      // set supervisor bit   
    page_table[video_start_idx].pwt = 0;
    page_table[video_start_idx].pcd  = 0;
    page_table[video_start_idx].accessed  = 0;
    page_table[video_start_idx].dirty = 0;
    page_table[video_start_idx].page_table_attribute_index = 0;
    page_table[video_start_idx].global_page = 0;
    page_table[video_start_idx].avail = 0;

    /* Starting index of Video Memory*/
    video_start_idx = (uint32_t)0xBB000 >> 12; // index should be at 184 out of 1024
    page_table[video_start_idx].present = 1;              //set present bit
    page_table[video_start_idx].read_write = 1;           // set read write bit
    page_table[video_start_idx].user_supervisor = 1;      // set supervisor bit   
    page_table[video_start_idx].pwt = 0;
    page_table[video_start_idx].pcd  = 0;
    page_table[video_start_idx].accessed  = 0;
    page_table[video_start_idx].dirty = 0;
    page_table[video_start_idx].page_table_attribute_index = 0;
    page_table[video_start_idx].global_page = 0;
    page_table[video_start_idx].avail = 0;

    /* Attach Page Table to the first index in the Page Directory*/
    int video_directory_idx = (uint32_t)VIDEO_MEM_START >> 22;          // examine 10 MSBs within virtual adress to find its index within page directory
    page_directory[video_directory_idx]._4k_pt.pt_base_address = ((uint32_t)page_table >> 12); // Store the page table's address in the page directory
    page_directory[video_directory_idx]._4k_pt.present = 1;             // set present bit
    page_directory[video_directory_idx]._4k_pt.read_write = 1;             // set present bit


    /* Attach second page table to the page directory */
    int new_video_directory_idx = (uint32_t)(USER_VIDMEM) >> 22;  // get 10 MSBs
    page_directory[new_video_directory_idx]._4k_pt.pt_base_address = ((uint32_t)page_table_new >> 12);  // address of page table
    page_directory[new_video_directory_idx]._4k_pt.user_supervisor = 1; // user privilege level
    page_directory[new_video_directory_idx]._4k_pt.present = 1;           // set present bit 
    page_directory[new_video_directory_idx]._4k_pt.read_write = 1;    

    /* Attach 4MB Kernel Page to the second index in the Page Directory */
    int _4m_directory_index = (uint32_t)(KERNEL_MEM_START >> 22);       // examine 10 MSBs within virtual adress to find its index within page directory 
    page_directory[_4m_directory_index]._4m_p.present = 1;              // set present bit
    page_directory[_4m_directory_index]._4m_p.read_write = 1;           // set read_write bit
    page_directory[_4m_directory_index]._4m_p.page_size = 1;            // set page_size bit
    page_directory[_4m_directory_index]._4m_p.global_page = 1;          // set global page bit
    page_directory[_4m_directory_index]._4m_p.page_base_address = (uint32_t)(KERNEL_MEM_START >> 22);       // since virtual memory maps to the same memory in physcical memory
                                                                                                            //  grab the 10 MSB of the Kernel's virtual memory to set as page's base address

    /* Set the Control Registers */
    set_control_registers((unsigned int*)page_directory);
    return;
}


/* 
 * load_user_program
 *   DESCRIPTION: Map virtual memory for the user program, which begins at 128
 *                MB in virtual memory, to physical memory. Frame location within 
 *                physical memory is dependent on the process number argument. 
 *                 
 *   INPUTS: uint32_t process_number -- acts as an offset indicator of where to place page in physical mem
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Maps User Program to Physical Memory
 */
void load_user_program(uint32_t process_number){
    uint32_t user_physical_mem;                                             // address in physical mem to map virtual user program 
    user_physical_mem = PHYS_USER_PROG_STR + (process_number * SIZE_4MB);   // get starting address of 4 MB page in physcal mem

    /* Attach 4MB User Page to the Page Directory */
    int _4m_user_index = (uint32_t)(VIRTUAL_USER_PROG >> 22);       // examine 10 MSBs within virtual adress to find its index within page directory 
    page_directory[_4m_user_index]._4m_p.present = 1;              // set present bit
    page_directory[_4m_user_index]._4m_p.user_supervisor = 1;
    page_directory[_4m_user_index]._4m_p.read_write = 1;           // set read_write bit
    page_directory[_4m_user_index]._4m_p.page_size = 1;            // set page_size bit
    page_directory[_4m_user_index]._4m_p.global_page = 0;         // set global page bit
    page_directory[_4m_user_index]._4m_p.pcd = 1;
    page_directory[_4m_user_index]._4m_p.pwt = 0;
    page_directory[_4m_user_index]._4m_p.page_base_address = (uint32_t)(user_physical_mem >> 22);    //  grab the 10 MSB of physical address to set plane
                                                                                                    
}
/* 
 * unload_user_program
 *   DESCRIPTION: Unmap frame associated with current process from physical memory.
 *                 
 *   INPUTS: uint32_t process_number -- acts as an offset indicator of where to unmap page in physical mem
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Unmaps process from Physical Memory
 */
 void unload_user_program(uint32_t process_number){
    uint32_t user_physical_mem;                                 // address in physical mem where to unmap virtual user program 
    user_physical_mem = PHYS_USER_PROG_STR + (process_number * SIZE_4MB);  // get starting address of 4 MB page in physcal mem

    int _4m_user_index = (uint32_t)(VIRTUAL_USER_PROG >> 22);       // examine 10 MSBs within virtual adress to find its index within page directory 
    page_directory[_4m_user_index]._4m_p.present = 0;              // set present bit to 0, unmap page
    page_directory[_4m_user_index]._4m_p.page_base_address = (uint32_t)(user_physical_mem >> 22);       //  grab the 10 MSB of physical address to set plane
                                                                                                           
 }

/* 
 * load_vidmem
 *   DESCRIPTION: Maps virtual memory for video memory to the physical memory for video memory,
 *                with user privilege level.
 *   INPUTS: uint8_t* screen_start -- virtual address of video memory
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Maps virtual video memory to physical video memory
 */
 void load_vidmem (uint8_t* screen_start) {     
    /* Initialize all pages within page table */
    int video_start_idx = ((uint32_t)screen_start >> 12) & TEN_LSB_MASK;   // get index into page table
    page_table_new[video_start_idx].present = 1;    // set present bit     
    page_table_new[video_start_idx].read_write = 1;          
    page_table_new[video_start_idx].user_supervisor = 1;    // user privilege level 
    page_table_new[video_start_idx].page_base_address =(uint32_t)(VIDEO_MEM_START >> 12);   // physical address of video memory
 }

/* 
 * unload_vidmem
 *   DESCRIPTION: Unmaps virtual memory for video memory to the physical memory for video memory.
 *   INPUTS: None 
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Unmaps virtual video memory
 */
 void unload_vidmem(){
    int video_start_idx = ((uint32_t)USER_VIDMEM >> 12) & TEN_LSB_MASK;  // get index into page table
    page_table_new[video_start_idx].present = 0;    // set present bit to 0
 }

/*
 * copy_to_vidmem()
 *   DESCRIPTION: Copies and saves terminals video memory
 *   INPUTS: old_terminal -- Terminal to be switching from
 *           new_terminal -- Terminal to be switching to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Copies and saves terminal video memory to correct location in memory
 */
void copy_to_vidmem (unsigned int old_terminal, unsigned int new_terminal) {
    /* Copy page contents and stuff */
    uint32_t terminal_index = TERMINAL_START + old_terminal*VIDMEM_SIZE;
    memcpy((int*)terminal_index , (void*)VIDEO_MEM_START, PAGE_4K_SIZE);
    terminal_index = TERMINAL_START + new_terminal*VIDMEM_SIZE;
    memcpy((void*)VIDEO_MEM_START, (int*)terminal_index, PAGE_4K_SIZE);

    /* Switch user program location*/
    // uint32_t user_physical_mem;
    // user_physical_mem = PHYS_USER_PROG_STR + (new_terminal * SIZE_4MB);
    // int _4m_user_index = (uint32_t)(VIRTUAL_USER_PROG >> 22);
    // page_directory[_4m_user_index]._4m_p.page_base_address = (uint32_t)(user_physical_mem >> 22);
}
