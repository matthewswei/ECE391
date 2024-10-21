/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
/* 
 * i8259_init()
 *   DESCRIPTION: Initializes I8259 PIC controller (master and slave)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initializes PIC controller to be used for interrupts
 */
void i8259_init(void) {
    //cli(); /* Enter critical section for PIC initialization */

    /* Cache or save before values of master and slave data ports */
    master_mask = 0xff; /* Mask for all 1s */
    slave_mask = 0xff; 

    /* Mask the master and slave data ports*/
    //outb(0xff, MASTER_8259_DATA_PORT); /* mask all of 8259A-1 */
    //outb(0xff, SLAVE_8259_DATA_PORT); /* mask all of 8259A-2 */

    /* Initializes the master PIC */
    outb(ICW1, MASTER_8259_PORT); /*ICW1: select 8259A-1 init */
    outb(ICW1, SLAVE_8259_PORT); /*ICW1: select 8259A-2 init */
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT); /*ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT); /*ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT); /*ICW3: 8259A-1 (primary) has secondary */
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT); /*ICW3: 8259A-2 is a secondary on primary */
    outb(ICW4, MASTER_8259_DATA_PORT);

    /* Initializes the slave PIC */
    outb(ICW4, SLAVE_8259_DATA_PORT);

    /* Restore the old values ot data ports */
    //outb(0xff, MASTER_8259_DATA_PORT);
    //outb(0xff, SLAVE_8259_DATA_PORT);

   // enable_irq(2);

    //sti(); /* End critical section */
}

/* Enable (unmask) the specified IRQ */
/* 
 * enable_irq()
 *   DESCRIPTION: Enables an IRQ slot on the PIC
 *   INPUTS: irq_num -- Interrupt request number for the PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables an IRQ slot on the PIC
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    /* Check if IRQ number is 8-15 (slave IRQs)*/
    if (irq_num > 7) {
        port = SLAVE_8259_DATA_PORT; /* Sets port to slave data port */
        irq_num-=8; /* Update IQR number*/
        value = slave_mask;
        value &= ~(1 << irq_num); /* Sets a 1 in the interrupt mask register (IMR) for specified IRQ */
        outb(value, port);
        slave_mask = value;
        port = MASTER_8259_DATA_PORT;
        value = master_mask;
        value &= ~(1 << 0x02); /* Turns on interrupts for port */
        outb(value, port);
        master_mask = value;
    } else {
        port = MASTER_8259_DATA_PORT; /* Sets port to master data port */
        value = master_mask;
        value &= ~(1 << irq_num);
        outb(value, port);
        master_mask = value;
    }

    // outb(value, port);
}

/* Disable (mask) the specified IRQ */
/* 
 * disable_irq()
 *   DESCRIPTION: Disables an IRQ slot on the PIC
 *   INPUTS: irq_num -- Interrupt request number for the PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Disables an IRQ slot on the PIC
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;
    /* Check if IRQ number is 8-15 (slave IRQs)*/
    if (irq_num > 7) {
        port = SLAVE_8259_DATA_PORT; /* Sets port to slave data port */
        irq_num-=8; /* Update IQR number*/
        value = slave_mask;
        value |= (1 << irq_num); /* Sets specified IRQ in the interrupt mask register (IMR) to 0 */
        outb(value, port);
        slave_mask = value;
        port = MASTER_8259_DATA_PORT;
        value = master_mask;
        value |= (1 << 0x02);
        outb(value, port);
        master_mask = value;
    } else {
        port = MASTER_8259_DATA_PORT; /* Sets port to master data port */
        value = master_mask;
        value |= (1 << irq_num); /* Sets specified IRQ in the interrupt mask register (IMR) to 0 */
        outb(value, port);
        master_mask = value;
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
/* 
 * send_eoi()
 *   DESCRIPTION: Sends an end-of-interrupt signal for the specified IRQ
 *   INPUTS: irq_num -- Interrupt request number for the PIC
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends an end-of-interrupt signal for the specified IRQ
 */
void send_eoi(uint32_t irq_num) {
    /* Check if IRQ number is 8-15 (slave IRQs)*/
    if (irq_num > 7) {
        irq_num-=8; /* Gets slave value port number */
        outb(EOI | irq_num, SLAVE_8259_PORT); /* Sends EOI command to slave PIC */
        outb(EOI | ICW3_SLAVE, MASTER_8259_PORT); /* Sends EOI command to master PIC (required because of cascading) */
    } else {
        outb(EOI | irq_num, MASTER_8259_PORT); /* Sends EOI command to master PIC */
    }
}
