/* keyboard.h - Defines used in interactions witht the keyboard
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

/* Variable declarations */
#define KEYBOARD_IRQ        1
#define KEYBOARD_DATA_PORT      0x60
#define SCAN_CODE_SIZE  59
#define MAX_Y   24
#define MAX_X   79
#define MAX_CHARS_TYPED 127
#define MAX_BUFFER_SIZE 128

/* Strcut to hold each terminals incormation separatly */
typedef struct terminal{
    int cur_x;
    int cur_y;
    int cur_index;
    char buffer[128];
    int address;
    int processes;
} terminal_t;

/* Scan code table */
extern unsigned char scan_code[SCAN_CODE_SIZE];

/* Scan code when shift is pressed */
extern unsigned char scan_code_SHIFT[SCAN_CODE_SIZE];

/* Scan code when caps lock is on */
extern unsigned char scan_code_CAPS[SCAN_CODE_SIZE];

/* Scan code when both shift and caps lock are pressed */
unsigned char scan_code_BOTH[SCAN_CODE_SIZE];

/* Initialize keyboard */
void keyboard_init();

/* Handler for keyboard interrupts */
void keyboard_handler();

/* Set flags in the case of shift, caps, or control */
void set_flags(int input);

/* Copies the contents of the keyboard buffer into the parameter buffer buf */
int32_t terminal_read (uint32_t fd, void* buf, uint32_t nbytes);

/* Always returns -1, for stdout purposes */
int32_t read_fail (uint32_t fd, void* buf, uint32_t nbytes);

/* Writes the contents of buf to the screen */
int32_t terminal_write (uint32_t fd, const void* buf, uint32_t nbytes);

/* Always returns -1, for stdin purposes */
int32_t write_fail (uint32_t fd, const void* buf, uint32_t nbytes);

/* Opens terminal driver */
int32_t terminal_open(const uint8_t* filename);

/* Always returns -1, for stdin and stdout purposes */
int32_t open_fail(const uint8_t* filename);

/* Closes terminal driver */
int32_t terminal_close(uint32_t fd);

/* Always returns -1, for stdin and stdout purposes */
int32_t close_fail(uint32_t fd);

/* Updates information in termianl */
void update_term_pid(int new_pid);

/* Gets current terminal being displayed */
int get_cur_term();

/* Updates terminal information upon switch */
void update_term(int term);

/* Get termianl PID from struct */
int get_term_pid(int term);

/* Increase process number in terminal */
void inc_term_proc(int term);

/* Decrease process number in terminal */
void dec_term_proc(int term);

/* Get number of processes in terminal */
int get_term_proc(int term);

#endif /* _KEYBOARD_H */
