/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/* local functions for file */
extern int tuxctl_ioctl (struct tty_struct* tty, struct file* file, unsigned cmd, unsigned long arg);
int tuxctl_init (struct tty_struct* tty);
int tuxctl_set_led (struct tty_struct* tty, unsigned long arg);
int tuxctl_buttons (struct tty_struct* tty, unsigned long arg);

/* local variables for file */
static spinlock_t lock = SPIN_LOCK_UNLOCKED;
int ack;

/* struct to hold tux controller information and status */
struct TUX_INFO {
	int buttons;
	int led;
} tux_info;

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */

/* Magic Numbers
 * 0x0F - 1111 in hex, used to separate 4 bits
 * 0x09 - 1001 in hex, used to get R and U values
 * 0x04 - 0100 in hex, used to get D value
 * 0x02 - 0010 in hex, used to get L value
 */

/* 
 * tuxctl_handle_packet()
 *   DESCRIPTION: Handles packets from TUX controller
 *   INPUTS: tty -- parameter used for tuxctl functions
 * 			 packet -- packet from TUX in [3][8] format
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sets ACK, buttons, or resets basaed on opcode
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned long buttons, arrows, flags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	/* Check packet information */
	switch (a) {
		case MTCP_ACK:
			ack = 1;
			return;
		case MTCP_BIOC_EVENT:
			/* Get button information from packets */
			buttons = ~b & 0x0F;
			arrows = ~c & 0x0F;
			arrows = (arrows & 0x09) | ((arrows & 0x04) >> 1) | ((arrows & 0x02) << 1);
			arrows = arrows << 4;
			/* Update tux status */
			spin_lock_irqsave(&lock, flags);
			tux_info.buttons = arrows | buttons;
			spin_unlock_irqrestore(&lock, flags);
			return;
		case MTCP_RESET:
			tuxctl_init(tty);
			if (ack) {
				tuxctl_set_led(tty, tux_info.led);
			}
			return;
		default:
			return;
	}

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/

/* 
 * tuxctl_ioctl
 *   DESCRIPTION: Main jump table for tuxctl ioctls
 *   INPUTS: tty -- parameter used for tuxctl functions
 * 	  	     file -- file input for the ioctl function
 *           cmd -- guide to which ioctl to run
 *           arg -- argument to be passed on to sub-functions
 *   OUTPUTS: none
 *   RETURN VALUE: return 0 if TUX_LED_ACK, TUX_LED_REQUEST, or TUX_READ_LED or
 *				   return value of other ioctl functions
 *   SIDE EFFECTS: Jumps to ioctl functions
 */
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT:
		return tuxctl_init(tty);
	case TUX_BUTTONS:
		return tuxctl_buttons(tty, arg);
	case TUX_SET_LED:
		return tuxctl_set_led(tty, arg);
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}

/* 
 * tuxctl_init
 *   DESCRIPTION: Initializes tux controller
 *   INPUTS: tty -- parameter used for tuxctl functions
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: Initializes tux controller
 */
int
tuxctl_init (struct tty_struct* tty) {
	unsigned char opcode[2] = {MTCP_BIOC_ON, MTCP_LED_USR};

	/* Initialize values */
	ack = 0;
	tux_info.buttons = 0x00;
	tux_info.led = 0x00;
	tuxctl_ldisc_put(tty, &opcode[0], 1);
	tuxctl_ldisc_put(tty, &opcode[1], 1);

	return 0;
}

/* Magic Numbers
 * 0x0F - 1111 in hex, used to get digit hex values
 * 0x01 - 1 in hex, used to check if specific led is on
 * 0x0F000000 - 1111000000000000000000000000 in hex, used to decimals
 * 0x000F0000 - 11110000000000000000 in hex, used to get which leds are on
 */

/* 
 * tuxctl_set_led
 *   DESCRIPTION: Sets LED values
 *   INPUTS: tty -- parameter used for tuxctl functions
 *     	     arg -- led information for digits
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on spam
 *   SIDE EFFECTS: sets and displays leds on tux
 */
int
tuxctl_set_led (struct tty_struct* tty, unsigned long arg) {
	unsigned char led_values[4];
 	unsigned char led_on;
 	unsigned char decimal;
 	unsigned int  i;		
 	unsigned long bit_check = 0x01;
	unsigned long bit_check_2 = 0x0F; 
 	unsigned char data_to_driver[6];
	unsigned char digit_data[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};

	/* Check for spam */
	if (!ack) {
		return -1;
	}
	ack = 0;

 	/* Get information from argument */
	led_on = (arg & 0x000F0000) >> 16;
	decimal = (arg & 0x0F000000) >> 24;

 	/* Get information from input */
 	for (i = 0; i<4; i++) {
 		led_values[i] = (bit_check_2 & arg) >> (4*i);
		/* Shift bit to check next digit */
		bit_check_2 = bit_check_2 << 4;
 	}

 	/* Change to User mode */
 	data_to_driver[0] = MTCP_LED_USR;
 	tuxctl_ldisc_put(tty, &data_to_driver[0], 1);

 	/* Set LED data to be printed */
 	data_to_driver[0] = MTCP_LED_SET;
 	data_to_driver[1] = 0x0F;

	/* Add information to be pushed onto the tux controller */
 	for (i = 0; i<4; i++) {
 		if(led_on & bit_check) {
 			led_values[i] = digit_data[led_values[i]];
 			if(decimal & bit_check)
 				led_values[i] |= 0x10; /* 10000 in hex which is where the decimal bit is*/
 			data_to_driver[2 + i] = led_values[i];
 		} else {
 			data_to_driver[2 + i] = 0x0;
 		}
		/* Shift bit to check next led spot */
		bit_check = bit_check << 1;
 	}

	/* Send to controller */
	tuxctl_ldisc_put(tty, data_to_driver, 6);
	tux_info.led = arg;

	return 0;
}

/* 
 * tuxctl_buttons
 *   DESCRIPTION: Sends button information to user-level
 *   INPUTS: tty -- parameter used for tuxctl functions
 *           arg -- Pointer to user-level address
 *   OUTPUTS: none
 *   RETURN VALUE: o if success and -EINVAL is non-valid input
 *   SIDE EFFECTS: Copies to user-level
 */
int
tuxctl_buttons (struct tty_struct* tty, unsigned long arg) {
	unsigned long flags;
	unsigned long kernel = (unsigned int)&tux_info.buttons;
	int bits_copied;

	/* Check pointer is valid */
	if (!arg) {
		return -EINVAL; 
	}

	/* Copy from kernel to user */
	spin_lock_irqsave(&lock, flags);
	bits_copied = copy_to_user((void *)arg, (void *)kernel, sizeof(long));
	spin_unlock_irqrestore(&lock, flags);

	/* Check if all bits are copied */
	if (bits_copied>0) {
		return -EINVAL;
	}

	return 0;
}
