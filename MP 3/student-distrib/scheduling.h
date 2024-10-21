#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "types.h"

// Function prototypes

/* Switch tasks from one process to another */
void switch_tasks();

/* Saves ESP value*/
extern uint32_t save_esp();

/* Saves EBP value */
extern uint32_t save_ebp();

/* Restores ESP and EBP values */
extern void restore_esp_ebp(uint32_t esp, uint32_t ebp);

/* Switch tasks from one process to another on termianl switch */
void switch_term_tasks(int start_pid, int end_pid);

/* Restores EBP value */
//extern void restore_ebp(uint32_t ebp);

#endif
