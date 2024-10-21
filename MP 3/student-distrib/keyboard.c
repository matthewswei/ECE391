/* keyboard.c - Functions to intitialize and handle keyboard and terminal
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "keyboard.h"
#include "paging.h"
#include "lib.h"
#include "syscalls.h"
#include "scheduling.h"

/* Scan code when neither shift or caps lock pressed */
unsigned char scan_code[SCAN_CODE_SIZE] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 
                                '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
                                'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 
                                'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};

/* Scan code when shift is pressed */
unsigned char scan_code_SHIFT[SCAN_CODE_SIZE] = {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
                                '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0',
                                'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\'', '~', '\0', '|', 
                                'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};

/* Scan code when caps lock is on */
unsigned char scan_code_CAPS[SCAN_CODE_SIZE] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
                                '\0', '\0', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P','[', ']', '\n', '\0',
                                'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',';', '\'', '`', '\0', '\\', 
                                'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};

/* Scan code when both shift and caps lock are pressed */
unsigned char scan_code_BOTH[SCAN_CODE_SIZE] = {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 
                                '\0', '\0', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', '\0',
                                'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\'', '~', '\0', '|',
                                'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};

/* Local Variables */
unsigned int shift_flag = 0; /* Flag indicating when shift is pressed */
unsigned int caps_flag = 0; /* Flag indicating when caps lock is on */
unsigned int ctrl_flag = 0; /* Flag indicating when control is pressed */
unsigned int end_y_flag = 0; /* Flag indicating we are at row 24 (last row of video memory, for scrolling purposes) */
unsigned int alt_flag = 0;   /* Flag indicating when alt is pressed */
unsigned int displayed_term = 0;
terminal_t terminals[3];
char buffer[128]; /* Keyboard buffer */
int cur_index = 0; /* Current index of the keyboard buffer */
volatile unsigned int buf_flag = 0; /* Flag indicating we have a '\n' character as input (stop reading) */
int read_flag = 1;
int first_use_term_one = 0;
int first_use_term_two = 0;

/* 
 * keyboard_init()
 *   DESCRIPTION: Initializes the keyboard device
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Turns on interrupts for IRQ 1 on the PIC
 */
void keyboard_init() {
    cli();
    int i;
    /* Disable cursor */
    outb(0x0A, 0x3D4);
    outb(0x20, 0x3D5);
    for (i = 0; i<3; i++) {
        char term_buffer[128];
        terminals[i].cur_x = 0;
        terminals[i].cur_y = 0;
        terminals[i].cur_index = 0;
        memcpy((void*)terminals[i].buffer, (const void*)term_buffer, 128);
        terminals[i].processes = 0;
        // if (i == 0) {
        //     terminals[i].address = 0;//0xB9000 + (i*0x1000);
        // }
        // else {
            terminals[i].address = -1;
        //}
    }
    /* Enables interrupts for the keyboard */
    enable_irq(KEYBOARD_IRQ);
    sti();
}

/* 
 * keyboard_handler()
 *   DESCRIPTION: Handles keyboard interrupts and inputs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints keyboard input onto screen
 *                 Sends End-of-Interrupt signal to IRQ 1 on PIC
 */
void keyboard_handler() {
    cli();
    int input = inb(KEYBOARD_DATA_PORT);      /* Get input from keyboard */
    // 3A is the scan code for caps lock
    if (input == 0x3A) {
        caps_flag = !caps_flag; // flip the caps lock from what it was previously if it is clicked
    } else if (input == 0x2A || input == 0x36) { // 2A ad 36 are the scan codes for the left shift and right shift press respectively
        shift_flag = 1;
    } else if (input == 0xAA || input == 0xB6) { // AA ad B6 are the scan codes for the left shift and right shift release respectively
        shift_flag = 0;
    } else if (input == 0x1D) { // 1D is the scan code for control press
        ctrl_flag = 1;
    } else if (input == 0x9D) { // 9D is the scan code for control release
        ctrl_flag = 0;
    } 
    else if (input == 0x38) { // 38 is the scan code for alt press 
        alt_flag = 1;
    }
    else if (input == 0xB8){ // B8 is the scan code for alt release 
        alt_flag = 0;
    }
    else if (input == 0x0E) { // 0E is the scan code for backspace
        // if we are backspacing, update x and y positions so we are at the proper location
        // first case is if we are at the leftmost part of the screen, at which point move up to the previous line
        if (cur_index > 0) {
            if (get_screen_x() == 0) {
                if (get_screen_y() == 0) {}
                else {
                    set_screen_y(get_screen_y() - 1);
                    set_screen_x(MAX_X);
                    putc('\0');
                    set_screen_y(get_screen_y() - 1);
                    set_screen_x(MAX_X);
                }
            } else { // second case is if we are in the middle of the screen, at which point just go back to the left by one position
                set_screen_x(get_screen_x() - 1);
                putc('\0');
                set_screen_x(get_screen_x() - 1);
            }
            if (read_flag == 1) {
                cur_index--;
            }
        }
    } else if (input == 0x0F) { // if we are tabbing, update x and y positions so we are at the proper location
        int i = 0;
        // handle moving the x position to the right by the correct amount
        for (i = 0; i < 4; i++) {
            putc(' ');

            if (read_flag == 1) {
                buffer[cur_index] = ' ';
                cur_index++;
            }
        }
    } else if (alt_flag == 1) {
        /* Checks if a F# key is pressed */
        if (input == 0x3B) {
            /* Checks if terminal needs to be switched*/
            if(displayed_term != 0) {
                int orig_term = displayed_term;
                displayed_term = 0; // Change terminal number

                /* Updates current screen position and index */
                terminals[orig_term].cur_x = get_screen_x();
                terminals[orig_term].cur_y = get_screen_y();
                terminals[orig_term].cur_index = cur_index;

                /* Saves buffer */
                memcpy((void*)terminals[orig_term].buffer, (const void*)buffer, MAX_BUFFER_SIZE);

                /* Sets screen position and index for new terminal */
                set_screen_x(terminals[0].cur_x);
                set_screen_y(terminals[0].cur_y);
                cur_index = terminals[0].cur_index;

                /* Copies buffer */
                memcpy((void*)buffer, (const void*)terminals[0].buffer, MAX_BUFFER_SIZE);


                copy_to_vidmem(orig_term, 0);  /* Copies and saves terminal video information */
                send_eoi(KEYBOARD_IRQ);
                switch_term_tasks(terminals[orig_term].address, terminals[0].address); /* Switch processes */
            }
        }
        else if (input == 0x3C) {
            if(displayed_term != 1) {
                int orig_term = displayed_term;
                displayed_term = 1;
                terminals[orig_term].cur_x = get_screen_x();
                terminals[orig_term].cur_y = get_screen_y();
                terminals[orig_term].cur_index = cur_index;
                memcpy((void*)terminals[orig_term].buffer, (const void*)buffer, MAX_BUFFER_SIZE);
                set_screen_x(terminals[1].cur_x);
                set_screen_y(terminals[1].cur_y);
                cur_index = terminals[1].cur_index;
                memcpy((void*)buffer, (const void*)terminals[1].buffer, MAX_BUFFER_SIZE);
                copy_to_vidmem(orig_term, 1);
                //send_eoi(KEYBOARD_IRQ);

                /* Checks if shell needs to be initializes (first time entering terminal) */
                if (first_use_term_one == 0) {
                    send_eoi(KEYBOARD_IRQ);
                    first_use_term_one = 1;
                    int ret = sys_exec((const uint8_t*)"shell");

                    /* Checks if shell can be executed */
                    if (ret == -1) {
                        first_use_term_one = 0;
                    }
                }
                else {
                    send_eoi(KEYBOARD_IRQ);
                    switch_term_tasks(terminals[orig_term].address, terminals[1].address);
                }
            }
        }
        else if (input == 0x3D) {
            if(displayed_term != 2) {
                int orig_term = displayed_term;
                displayed_term = 2;
                terminals[orig_term].cur_x = get_screen_x();
                terminals[orig_term].cur_y = get_screen_y();
                terminals[orig_term].cur_index = cur_index;
                memcpy((void*)terminals[orig_term].buffer, (const void*)buffer, MAX_BUFFER_SIZE);
                set_screen_x(terminals[2].cur_x);
                set_screen_y(terminals[2].cur_y);
                cur_index = terminals[2].cur_index;
                memcpy((void*)buffer, (const void*)terminals[2].buffer, MAX_BUFFER_SIZE);
                copy_to_vidmem(orig_term, 2);
                //send_eoi(KEYBOARD_IRQ);
                if (first_use_term_two == 0) {
                    send_eoi(KEYBOARD_IRQ);
                    first_use_term_two = 1;
                    int ret = sys_exec((const uint8_t*)"shell");
                    if (ret == -1) {
                        first_use_term_two = 0;
                    }
                }
                else {
                    send_eoi(KEYBOARD_IRQ);
                    switch_term_tasks(terminals[orig_term].address, terminals[2].address);
                }
            }
        }
    } else if (input <= SCAN_CODE_SIZE) { // case for printing characters
        // clear the screen
        if (ctrl_flag == 1) {
            if (input == 0x26) { // 26 is the scancode for the letter 'l'
                clear();
                set_screen_x(0);
                set_screen_y(0);
            }
        } else if ((caps_flag == 1 && shift_flag == 1)) { // if caps and shift are pressed, read from BOTH scan code
            unsigned char printed  = scan_code_BOTH[input];
            putc(printed);      /* Prints char */
            if (read_flag == 1) {
                buffer[cur_index] = printed;
                cur_index++;
            }
        } else if (shift_flag == 1 && caps_flag == 0) { // if caps is not pressed and shift is pressed, read from SHIFT scan code
            unsigned char printed  = scan_code_SHIFT[input]; 
            putc(printed);      /* Prints char */
            if (read_flag == 1) {
                buffer[cur_index] = printed;
                cur_index++;
            }
        } else if ((shift_flag == 0 && caps_flag == 0)){ // if caps and shift are not pressed, read from original scan code
            unsigned char printed  = scan_code[input];
            putc(printed);      /* Prints char */     
            if (read_flag == 1) {
                buffer[cur_index] = printed;
                cur_index++;
            }     
        } else if (caps_flag == 1 && shift_flag == 0) { // if only caps is pressed, read from CAPS scan code
            unsigned char printed  = scan_code_CAPS[input];
            putc(printed);      /* Prints char */
            if (read_flag == 1) {
                buffer[cur_index] = printed;
                cur_index++;
            }
        } else { // edge case in case we fit none of the previous cases
            unsigned char printed  = scan_code[input];
            putc(printed);      /* Prints char */
            if (read_flag == 1) {
                buffer[cur_index] = printed;
                cur_index++;
            }
        }
        sti();
    }
    /* Sends end-of-interrupt signal */
    send_eoi(KEYBOARD_IRQ);
    /* Ends read if a newline character was pressed */
    if (input == 0x1C) {
        buf_flag = 1;
    }
    if (cur_index == 127) {
        read_flag = 0;
    }
}

/*
 * terminal_read()
 *   DESCRIPTION: Copies the contents of the keyboard buffer into the parameter buffer buf
 *   INPUTS: buf -- the buffer to copy the keyboard input to
 *           nbytes -- the number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes read
 *   SIDE EFFECTS: Updates buf with the contents of the keyboard (up to 128 characters or
 *                 newline character)
 */
int32_t terminal_read(uint32_t fd, void* buf, uint32_t nbytes) {
    buf_flag = 0;
    while (buf_flag == 0);  /* Wait until a newline character is pressed */
    int i = 0;
    int buf_length;
    int ret;
    char* buff = (char*)buf;
    /* Check if the keyboard buffer is greater than 127 characters */
    if (cur_index == MAX_CHARS_TYPED) {
        buf_length = MAX_CHARS_TYPED;   /* If it was, only copy the first 127 characters */
        ret = buf_length+1;
        buff[MAX_CHARS_TYPED] = '\n';    /* Set the last character to a newline character */
        buff[MAX_BUFFER_SIZE] = '\0';    /* Make the buffer null terminated */
    } else {
        buf_length = cur_index; /* If not, length of buf is length of the keyboard buffer */
        ret = buf_length;
        buff[buf_length] = '\0'; /* Make buffer null terminated */
    }
    /* Copy contents of buffer to buf */
    for (i = 0; i < buf_length; i++) {
        buff[i] = buffer[i];
    }
    buf = (void*)buff;
    cur_index = 0;  /* Clear keyboard buffer for next read */
    buf_flag = 0;   /* Clear buffer flag for next read */
    read_flag = 1;
    return ret; /* Return number of bytes read */
}

/*
 * terminal_write()
 *   DESCRIPTION: Writes the contents of buf to the screen
 *   INPUTS: buf -- the buffer print to the screen
 *           nbytes -- the number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes written
 *   SIDE EFFECTS: Prints the contents of buf to the screen
 */
int32_t terminal_write(uint32_t fd, const void* buf, uint32_t nbytes) {
    char* buff = (char*)buf;
    /* Check if input buffer is greater than 128 characters */
    // if (strlen(buff) > MAX_BUFFER_SIZE) {
    //     return -1;  /* If it is, return -1 to indicate failure */
    // }
    if (nbytes < 0) {
        return -1;
    }
    // if (nbytes > strlen(buff)) {
    //     return -1;
    // }
    int i = 0;
    /* Print out all characters in buf, scroll if necessary */
    while(i < nbytes) {
        putc(buff[i]);
        i++;
    }
    return nbytes;  /* Return the number of bytes written */
}

/*
 * terminal_open()
 *   DESCRIPTION: Does nothing
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: always returns 0
 *   SIDE EFFECTS: Does nothing, returns 0
 */
int32_t terminal_open(const uint8_t* filename) {
    return 0;
}

/*
 * terminal_close()
 *   DESCRIPTION: Does nothing
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: always returns -1
 *   SIDE EFFECTS: Does nothing, returns -1
 */
int32_t terminal_close(uint32_t fd) {
    return -1;
}

/*
 * read_fail()
 *  Description: Always returns -1, for stdout purposes
 *  Inputs: none
 *  Outputs: none
 *  Return value: -1
 *  Side effects: none
 */
int32_t read_fail (uint32_t fd, void* buf, uint32_t nbytes) {
    return -1;
}

/*
 * write_fail()
 *  Description: Always returns -1, for stdin purposes
 *  Inputs: none
 *  Outputs: none
 *  Return value: -1
 *  Side effects: none
 */
int32_t write_fail (uint32_t fd, const void* buf, uint32_t nbytes) {
    return -1;
}

/*
 * open_fail()
 *  Description: Always returns -1, for stdin and stdout purposes
 *  Inputs: none
 *  Outputs: none
 *  Return value: -1
 *  Side effects: none
 */
int32_t open_fail(const uint8_t* filename) {
    return -1;
}

/*
 * close_fail()
 *  Description: Always returns -1, for stdin and stdout purposes
 *  Inputs: none
 *  Outputs: none
 *  Return value: -1
 *  Side effects: none
 */
int32_t close_fail(uint32_t fd) {
    return -1;
}

/*
 * update_term_pid()
 *   DESCRIPTION: Updates the current process for a terminal
 *   INPUTS: new_pid -- new pid number to be switched to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Updates terminal struct
 */
void update_term_pid(int new_pid) {
    terminals[displayed_term].address = new_pid;
}

/*
 * get_cur_term()
 *   DESCRIPTION: Returns current number of current terminal open
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: Current terminal being shown
 *   SIDE EFFECTS: none
 */
int get_cur_term() {
    return displayed_term;
}

/*
 * update_term()
 *   DESCRIPTION: Updates terminal information upon switch
 *   INPUTS: term -- Terminal number to change information from 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes values in terminal struct
 */
void update_term(int term) {
    terminals[displayed_term].cur_x = get_screen_x();
    terminals[displayed_term].cur_y = get_screen_y();
    terminals[displayed_term].cur_index = cur_index;
    memcpy((void*)terminals[term].buffer, (const void*)buffer, MAX_BUFFER_SIZE);
    set_screen_x(terminals[term].cur_x);
    set_screen_y(terminals[term].cur_y);
    cur_index = terminals[term].cur_index;
    memcpy((void*)buffer, (const void*)terminals[term].buffer, MAX_BUFFER_SIZE);
    copy_to_vidmem(displayed_term, term);
    displayed_term = term;
}

/*
 * get_term_pid()
 *   DESCRIPTION: Returns terminal's process number
 *   INPUTS: term -- Terminal number to get number from
 *   OUTPUTS: none
 *   RETURN VALUE: Process number
 *   SIDE EFFECTS: none
 */
int get_term_pid(int term) {
    return terminals[term].address;
}

/*
 * inc_term_proc()
 *   DESCRIPTION: Increases number of processes in current terminal
 *   INPUTS: term -- Terminal to increase processes in
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Increments number of processes in terminal
 */
void inc_term_proc(int term) {
    terminals[term].processes++;
}

/*
 * dec_term_proc()
 *   DESCRIPTION: Decreases number of processes in current terminal
 *   INPUTS: term -- Terminal to decrease processes in
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Decrements number of processes in terminal
 */
void dec_term_proc(int term) {
    terminals[term].processes--;
}

/*
 * get_term_proc()
 *   DESCRIPTION: Returns the number of processes present in current terminal
 *   INPUTS: term -- Current terminal to get processes from
 *   OUTPUTS: none
 *   RETURN VALUE: Number of processes being run on terminal
 *   SIDE EFFECTS: none
 */
int get_term_proc(int term) {
    return terminals[term].processes;
}

