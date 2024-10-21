#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "scheduling.h"
#include "keyboard.h"
#include "paging.h"

/* Locat variables */
int displayed = 0;

/* 
 * pit_init()
 *   DESCRIPTION: Initializes programmable interval timer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes the PIT to a square wave generator with a frequency of 60 Hz
 */
void pit_init() {
    // Init function is mainly sourced from http://www.osdever.net/bkerndev/Docs/pit.htm 
    //                                  and https://wiki.osdev.org/Programmable_Interval_Timer

    uint32_t Hz = 100; // wanted freq between 20 Hz and 100 Hz so 60 Hz works
    uint32_t divisor = 1193180 / Hz;       /* Calculate our divisor */

    // Disable interrupts
    //cli();

    // 0x43 is Mode/Command register (write only, a read is ignored)
    // 0x36 is the resultant byte that we should set in the Command register
    outb(0x36, 0x43);             /* Set our command byte 0x36 */

    // 0x40 is Channel 0 data port (read/write)
    outb((uint8_t) (divisor & 0xFF), 0x40);   /* Set low byte of divisor */
    outb((uint8_t) ((divisor >> 8) & 0xFF), 0x40);     /* Set high byte of divisor, mask with 0xFF since we only want the low 8 bits */

    // Enables IRQ slot 0 on the PIC
    enable_irq(0);

    // Enable interrupts
    //sti();

}

/* 
 * pit_handler()
 *   DESCRIPTION: handles which task to switch to based on the pit
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: switches the current task to the next task of the round robin
 */
void pit_handler(){
    // putc('a');
    // update_term((displayed++) % 3);
    send_eoi(0);
    switch_tasks();


}

