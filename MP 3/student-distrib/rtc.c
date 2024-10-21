/* rtc.c - Functions to intitialize RTC
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "rtc.h"
#include "lib.h"

/* Local variables */
volatile int flag = 1;

/* 
 * rtc_init()
 *   DESCRIPTION: Initializes Real Time Clock
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Turns on and sets RTC registers
 *                 Enables IRQ 8 on PIC
 */
void rtc_init() {
    uint8_t prev;
    uint8_t value;

    /* Set values for status register a */
    outb(STATUS_REG_A, RTC_PORT);
    prev = inb(CMOS_PORT);
    value = (SET_BIT_CHECK & prev) | A_INIT;
    outb(STATUS_REG_A, RTC_PORT);
    outb(value, CMOS_PORT);

    /* Set values for status register b */
    outb(STATUS_REG_B, RTC_PORT);
    prev = inb(CMOS_PORT);
    outb(STATUS_REG_B, RTC_PORT);
    value = prev | B_INIT;
    outb(value, CMOS_PORT);

    // const uint8_t* filename; /****TEST RTC_OPEN****/
    // rtc_open(filename);

    // int* buffer;
    // buffer[0] = 2;
    // rtc_write(0, buffer, 4); /****TEST RTC_WRITE****/

    /* Enables interrupts for the RTC */
    enable_irq(RTC_IRQ);

    // while(1){
    //     test_interrupts();
    // }
}

/* 
 * rtc_handler()
 *   DESCRIPTION: Handles interrupts for the Real-Time Clock
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clears Status Register C on the Real-Time Clock
 *                 Sends End-of-Interrupt signal to IRQ 8 on PIC
 */
void rtc_handler() {
    flag = 0;   /* Interrups has occurred, so set flag to 0 */
    outb(STATUS_REG_C, RTC_PORT);   /* Set register to status register c */
    inb(CMOS_PORT);     /* Clear status register c */
    // printf("...");   /* Make sure this function is being called */
    send_eoi(RTC_IRQ);      /* Send end-of-interrupt signal */
}

/* 
 * rtc_read()
 *   DESCRIPTION: Waits until an interrupt has occurred, then returns 0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: always returns 0
 *   SIDE EFFECTS: none
 */
int32_t rtc_read (uint32_t fd, void* buf, uint32_t nbytes) {
    while(flag != 0);   /* Wait until an interrupt has occurred (and RTC is not updating) */
    flag = 1;   /* Set flag back to 1 now that we have finished waiting */
    // printf("o");
    return 0;   /* Return 0 after escaping while loop */
}

/* 
 * rtc_write()
 *   DESCRIPTION: Changes the frequency of the RTC
 *   INPUTS: buf -- the frequency to set the RTC to
 *           nbytes -- the number of bytes of the interrupt rate (should always be 4)
 *   OUTPUTS: none
 *   RETURN VALUE: returns the number of bytes written, or -1 on failure
 *   SIDE EFFECTS: Changes the frequency of the RTC
 */
int32_t rtc_write (uint32_t fd, const void* buf, uint32_t nbytes){
    /* Make sure the input buffer is not NULL */
    if (buf == NULL) {
        return -1;  /* If it is NULL, return -1 */
    }
    int freq = *(int*)buf;
    /* Make sure the input frequency is a power of 2 */
    if ((freq & (freq-1)) != 0) {
        return -1;  /* If it is not, return -1 */
    }
    /* Make sure the input frequency is <= 1024 */
    if (freq > MAX_FREQ) {
        return -1;  /* If it is not, return -1 */
    }
    /* Make sure nbytes parameter is 4 */
    if (nbytes != 4) {
        return -1;  /* If it is not, return -1 */
    }
    int rate = 0;   /* Rate from the frequency equation (frequency = 32768 >> (rate - 1)) */
    int check = 0x8000; /* 32768 in hex (from above equation) */
    /* Right shift check until it equals the input to calculate rate - 1 */
    while (check != freq) {
        check /= 2;
        rate++;
    }
    rate++; /* Add 1 because rate holds rate - 1 after while loop */
    rate &= 0x0F;   /* Get the low 4 bits of rate (cannot be greater than 15) */
    cli();  /* Clear interrupts */
    /* Write the calculated rate to status register A */
    outb(STATUS_REG_A, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(STATUS_REG_A, RTC_PORT);
    outb((prev & 0xF0) | rate, CMOS_PORT);
    sti();  /* Enable interrupts */
    return 0;
}

/* 
 * rtc_open()
 *   DESCRIPTION: Resets the RTC frequency to 2Hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 on success
 *   SIDE EFFECTS: resets the RTC frequency to 2Hz
 */
int32_t rtc_open (const uint8_t* filename){
    /* Make sure the input filename is not NULL */
    if (filename == NULL) {
        return -1;  /* If it is NULL, return -1 */
    }
    int rate = TWOHZ_RATE;  /* Set rate to 15 (so that frequency is 2Hz) */
    rate &= 0x0F;   /* Get the low bits of rate */
    cli();  /* Clear interrupts */
    /* Write the rate to status register A */
    outb(STATUS_REG_A, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(STATUS_REG_A, RTC_PORT);
    outb((prev & 0xF0) | rate, CMOS_PORT);
    sti();  /* Enable interrupts */
    return 0;
}

/* 
 * rtc_close()
 *   DESCRIPTION: Does nothing
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: returns 0 on success
 *   SIDE EFFECTS: none
 */
int32_t rtc_close (uint32_t fd){
    return 0;   /* Does nothing, just return 0 */
}
