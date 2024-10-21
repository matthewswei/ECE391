/* idt.c - Functions to initialize the Interrupt Descriptor Table (IDT)
 * vim:ts=4 noexpandtab
 */

#include "idt.h"
#include "keyboard.h"
#include "rtc.h"
#include "lib.h"
#include "x86_desc.h"
#include "linkage.h"
#include "syscalls.h"

/* ****FOR REFERENCE**** */
/* typedef union idt_desc_t {
    uint32_t val[2];
    struct {
        uint16_t offset_15_00;
        uint16_t seg_selector;
        uint8_t  reserved4;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
        uint32_t size      : 1;
        uint32_t reserved0 : 1;
        uint32_t dpl       : 2;
        uint32_t present   : 1;
        uint16_t offset_31_16;
    } __attribute__ ((packed));
} idt_desc_t; */


/*
 * void setup_idt(void);
 * Inputs: void
 * Return Value: none
 * Function: Initializes the IDT with gate descriptors, and sets the offsets for
 *           each gate descriptor to point to its handler linkage function
 */
void setup_idt() {
    union idt_desc_t gate;  /* Declare a structure for an IDT descriptor */
    int i = 0;
    /* Iterate through all 255 descriptors in the IDT */
    for (i = 0; i < NUM_VEC; i++) {
        gate.seg_selector = KERNEL_CS;
        gate.reserved4 = 0x00;
        /* Check if the current index is an exception */
        if (i < NUM_EXC || i == SYS_INDEX) {
            gate.reserved3 = 1; /* If it is, use a Trap gate */
        } else {
            gate.reserved3 = 0; /* Otherwise, use an interrupt gate */
        }
        gate.reserved2 = 1;
        gate.reserved1 = 1;
        gate.size = 1;
        gate.reserved0 = 0;
        /* Check if the index is an exception or keyboard/rtc interrupt */
        if (i < NUM_EXC || i == KEYBOARD_INDEX || i == RTC_INDEX || i == PIT_INDEX) {
            gate.dpl = 0;   /* If it is, set privilege level to 0 */
        } else {
            gate.dpl = 3;   /* Otherwise, set to 3 */
        }
        /* Check if index is an exception we have a handler for, or a keyboard/rtc interrupt */
        if ((i < NUM_OUR_EXC && i != RESERVED) || i == KEYBOARD_INDEX || i == RTC_INDEX || i == SYS_INDEX || i == PIT_INDEX) {
            gate.present = 1;   /* If it is, set present bit to 1 */
        } else {
            gate.present = 0;   /* Otherwise, set to 0 */
        }
        idt[i] = gate;
    }

    /* Set the entries for the IDT exceptions to the corresponding handler */
    SET_IDT_ENTRY(idt[0], handle_DE);
    SET_IDT_ENTRY(idt[1], handle_DB);
    SET_IDT_ENTRY(idt[2], handle_NMI);
    SET_IDT_ENTRY(idt[3], handle_BP);
    SET_IDT_ENTRY(idt[4], handle_OF);
    SET_IDT_ENTRY(idt[5], handle_BR);
    SET_IDT_ENTRY(idt[6], handle_UD);
    SET_IDT_ENTRY(idt[7], handle_NM);
    SET_IDT_ENTRY(idt[8], handle_DF);
    SET_IDT_ENTRY(idt[9], handle_CSO);
    SET_IDT_ENTRY(idt[10], handle_TS);
    SET_IDT_ENTRY(idt[11], handle_NP);
    SET_IDT_ENTRY(idt[12], handle_SS);
    SET_IDT_ENTRY(idt[13], handle_GP);
    SET_IDT_ENTRY(idt[14], handle_PF);
    SET_IDT_ENTRY(idt[16], handle_MF);
    SET_IDT_ENTRY(idt[17], handle_AC);
    SET_IDT_ENTRY(idt[18], handle_MC);
    SET_IDT_ENTRY(idt[19], handle_XF);

    /* set the entry for index 33 of the IDT to the keyboard handler linkage */
    SET_IDT_ENTRY(idt[KEYBOARD_INDEX], kb_linkage);

    /* set the entry for index 34 of the IDT to the RTC handler linkage */
    SET_IDT_ENTRY(idt[RTC_INDEX], rtc_linkage);

    SET_IDT_ENTRY(idt[SYS_INDEX], system_call);

    SET_IDT_ENTRY(idt[PIT_INDEX], pit_linkage);
}

/*
 * void handle_DE(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_DE() {
    printf("Exception - Divide by zero\n");
    sys_halt (255);
    return;
}

/*
 * void handle_DB(void);
 * Inputs: void
 * Return Value: none
 * Function: Intel reserved, does nothing
 */
void handle_DB() {
    /* for Intel use only */
    printf("Exception - DB\n");
    sys_halt (255);
    return;
}

/*
 * void handle_NMI(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_NMI() {
    printf("Exception - Nonmaskable external interrupt\n");
    sys_halt (255);
    return;
}

/*
 * void handle_BP(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_BP() {
    printf("Exception - Breakpoint\n");
    sys_halt (255);
    return;
}

/*
 * void handle_OF(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_OF() {
    printf("Exception - Overflow\n");
    sys_halt (255);
    return;
}

/*
 * void handle_BR(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_BR() {
    printf("Exception - Bound range exceeded\n");
    sys_halt (255);
    return;
}

/*
 * void handle_UD(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_UD() {
    printf("Exception - Invalid opcode\n");
    sys_halt (255);
    return;
}

/*
 * void handle_NM(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_NM() {
    printf("Exception - Device not available\n");
    sys_halt (255);
    return;
}

/*
 * void handle_DF(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_DF() {
    printf("Exception - Double fault\n");
    sys_halt (255);
    return;
}

/*
 * void handle_CSO(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_CSO() {
    printf("Exception - Coprocessor segment overrun\n");
    sys_halt (255);
    return;
}

/*
 * void handle_TS(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_TS() {
    printf("Exception - Invalid TSS\n");
    sys_halt (255);
    return;
}

/*
 * void handle_NP(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_NP() {
    printf("Exception - Segment not present\n");
    sys_halt (255);
    return;
}

/*
 * void handle_SS(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_SS() {
    printf("Exception - Stack-segment fault\n");
    sys_halt (255);
    return;
}

/*
 * void handle_GP(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_GP() {
    printf("Exception - General protection\n");
    sys_halt (255);
    return;
}

/*
 * void handle_PF(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_PF() {
    printf("Exception - Page fault\n");
    sys_halt (255);
    return;
}

/*
 * void handle_MF(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_MF() {
    printf("Exception - x87 FPU floating-point error\n");
    sys_halt (255);
    return;
}

/*
 * void handle_AC(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_AC() {
    printf("Exception - Alignment check\n");
    sys_halt (255);
    return;
}

/*
 * void handle_MC(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_MC() {
    printf("Exception - Machine check\n");
    sys_halt (255);
    return;
}

/*
 * void handle_XF(void);
 * Inputs: void
 * Return Value: none
 * Function: Prints a statement describing the exception that occurred
 */
void handle_XF() {
    printf("Exception - SIMD floating-point exception\n");
    sys_halt (255);
    return;
}
