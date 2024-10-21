/* idt.h - Defines various functions and variables needed for the IDT
 * vim:ts=4 noexpandtab
 */

#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"

/* Local Variables */
#define KEYBOARD_INDEX  33  /* Index in the IDT for keyboard interrupts. */
#define RTC_INDEX   40  /* Index in the IDT for RTC interrupts */
#define RESERVED    15  /* Exception reserved for Intel */
#define NUM_EXC 32  /* Number of exceptions */
#define NUM_OUR_EXC 20  /* Number of exceptions not Intel reserved */
#define SYS_INDEX   128
#define PIT_INDEX   0x20

/* Initialize the IDT with gate descriptors and corresponding handlers. */
void setup_idt();

/* Exception Handlers */
void handle_DE(); /* Divide by zero exception handler. */
void handle_DB(); /* DB exception handler (for Intel use only). */
void handle_NMI(); /* Nonmaskable interrupt exception handler. */
void handle_BP(); /* Breakpoint exception handler. */
void handle_OF(); /* Overflow exception handler. */
void handle_BR(); /* BOUND range exceeded exception handler. */
void handle_UD(); /* Invalid opcode exception handler. */
void handle_NM();  /* Device not available exception handler. */
void handle_DF(); /* Double fault exception handler. */
void handle_CSO(); /* Coprocessor segment overrun exception handler. */
void handle_TS(); /* Invalid TSS exception handler. */
void handle_NP(); /* Segment not present exception handler. */
void handle_SS(); /* Stack segment fault exception handler. */
void handle_GP(); /* General protection exception handler. */
void handle_PF(); /* Page fault exception handler. */
void handle_MF(); /* x87 FPU floating-point error exception handler. */
void handle_AC(); /* Alignment check exception handler. */
void handle_MC(); /* Machine check exception handler. */
void handle_XF(); /* SIMD floating-point exception handler. */

#endif /* _IDT_H */
