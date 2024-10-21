#include "scheduling.h"
#include "syscalls.h"
#include "x86_desc.h"
#include "paging.h"
#include "i8259.h"
#include "lib.h"


/* Local Variables */
int cur_term = 0;

/*
 * switch_tasks()
 *   DESCRIPTION: Switches process being run upon terminal switch
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Chances the process to current process on new terminal
 */
void switch_tasks(){
    // //uint32_t saved_esp, saved_ebp;
    // int cur_pid = get_term_pid(cur_term);
    // int original = cur_term;
    // int next_pid = -1;
    // while (next_pid == -1) {
    //     cur_term++;
    //     cur_term %= 3;
    //     if (cur_term == original) {
    //         return;
    //     }
    //     next_pid = get_term_pid(cur_term);
    // }
    // pcb_t* pcb = (pcb_t*)get_pcb_from_pid(cur_pid);
    // //int32_t cur_pid = pcb->pid;

    // //int32_t next_pid = (cur_scheduled + 1) % 6;
    // pcb_t* new_pcb = (pcb_t*)get_pcb_from_pid(next_pid);

    // // Save esp/ebp
    // uint32_t saved_esp = save_esp();
    // pcb->esp = saved_esp;
    // uint32_t saved_ebp = save_ebp();
    // pcb->ebp = saved_ebp;

    // // Switch process paging
    // unload_user_program(cur_pid);
    // load_user_program(next_pid);

    // // Set TSS
    // tss.esp0 =  EIGHT_MB - (next_pid * EIGHT_KB) - PADDING; // kernel stack pointer
    // tss.ss0 = KERNEL_DS;    // kernel data segment (stack segment)

    // // Update running video coordinates


    // // Restore next process' esp/ebp
    // restore_esp_ebp(new_pcb->esp, new_pcb->ebp);

}

/*
 * switch_term_tasks()
 *   DESCRIPTION: Switches tasks to new terminal upon switch
 *   INPUTS: start_pid -- Current terminal's PID number
 *           end_pid -- New switched terminal's PID number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Loads the switched terminals process, replacing the old one's
 */
void switch_term_tasks(int start_pid, int end_pid) {
    // cli();
    // pcb_t* pcb = (pcb_t*)get_pcb_from_pid(start_pid);
    // if((int)pcb == -1) {return;}
    // pcb_t* new_pcb = (pcb_t*)get_pcb_from_pid(end_pid);
    // if((int)new_pcb == -1) {return;}
    // // Save esp/ebp
    // register uint32_t saved_esp asm("esp");
    // pcb->esp = saved_esp;
    // register uint32_t saved_ebp asm("ebp");
    // pcb->ebp = saved_ebp;

    // // Switch process paging
    // unload_user_program(start_pid);
    // load_user_program(end_pid);

    // // Set TSS
    // tss.esp0 =  EIGHT_MB - (end_pid * EIGHT_KB) - PADDING; // kernel stack pointer
    // tss.ss0 = KERNEL_DS;    // kernel data segment (stack segment)

    // // Update running video coordinates

    // flush_tlb();
    // // Restore next process' esp/ebp
    // sti();
    // restore_esp_ebp(new_pcb->esp, new_pcb->ebp);
}





