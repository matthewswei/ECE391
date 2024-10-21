/* rtc.h - Defines used in interactions with RTC
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

/* RTC and CMOS port values */
#define RTC_PORT    0x70
#define CMOS_PORT    0x71

/* IRQ value for RTC */
#define RTC_IRQ     8

/* Status register port values */
#define STATUS_REG_A    0x8A
#define STATUS_REG_B    0x8B
#define STATUS_REG_C    0x0C

/* Initial values for status registers */
#define A_INIT  0x26
#define B_INIT  0x40

/* Bit mask for checking bit 7 */
#define SET_BIT_CHECK   0x80

/* Max frequency for RTC */
#define MAX_FREQ    1024

/* Rate corresponding to 2Hz frequency */
#define TWOHZ_RATE  15

/* Initialize RTC */
void rtc_init();
/* Handle RTC Interrupts */
void rtc_handler();
/* Reads Data */
int32_t rtc_read (uint32_t fd, void* buf, uint32_t nbytes);
/* Writes Data */
int32_t rtc_write (uint32_t fd, const void* buf, uint32_t nbytes);
/* Provide Access to File System */
int32_t rtc_open (const uint8_t* filename);
/* Closes File Descriptor */
int32_t rtc_close (uint32_t fd);

#endif  /* _RTC_H */
