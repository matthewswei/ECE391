#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "rtc.h"
#include "file_system_driver.h"
#include "keyboard.h"
#include "syscalls.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */

 /*  					CHECKPOINT 1  TESTS  BEGIN              */

int idt_test(){
	int i;
	TEST_HEADER;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// EXCEPTION HANDLER TEST CASE
/*
 * int div_by_0();
 * Inputs: void
 * Return Value: FAIL if test case fails
 * Function: Should result in an exception if successful
 */
// int div_by_0(){
// 	int j;
// 	j = 2 / 0;
// 	return FAIL;
// }

// PAGING TEST CASES BEGIN

/*
 * int dereferencing_kernel();
 * Inputs: void
 * Return Value: PASS if test case passes
 * Function: test case will pass if we are able to dereference kernel memory
 */
// int dereferencing_kernel(){
// 	char success;
// 	char* address;
// 	address = (char *)0x400000; // kernel is loaded at physical address
// 	success = *address;

// 	address = (char *)0x7FFFFF; // end of kernel memory
// 	success = *address;

// 	return PASS; // this line will only be reached if we dereference kernel address successfully
// }

// /*
//  * int dereferencing_video_memory();
//  * Inputs: void
//  * Return Value: PASS if test case passes
//  * Function: test case will pass if we are able to dereference video memory
//  */
// int dereferencing_video_memory(){
// 	char success;
// 	char* address;
// 	address = (char *)0xB8000; // video memory is loaded at physical address
// 	success = *address;

// 	address = (char *)0xC0000; // end of video memory
// 	success = *address;

// 	return PASS; // this line will only be reached if we dereference video memory address successfully
// }

// /*
//  * int dereferencing_in_between();
//  * Inputs: void
//  * Return Value: PASS if test case passes
//  * Function: test case will pass if we are able to dereference at memory address between kernel beginning and end and video memory beginning and end
//  */
// int dereferencing_in_between(){
// 	char success;
// 	char* address;
// 	address = (char *)0xB8555; // in between video memory
// 	success = *address;

// 	address = (char *)0x555555; // in between kernel memory
// 	success = *address;

// 	return PASS; // this line will only be reached if we dereference video memory address successfully
// }

// /*
//  * int dereferencing_before();
//  * Inputs: void
//  * Return Value: PASS if test case passes
//  * Function: test case will pass if we are able to dereference right before memory address for kernel beginning and right before video memory beginning
//  */
// int dereferencing_before_and_after(){
// 	char success;
// 	char* address;
// 	address = (char *)0xB7FF; // before video memory
// 	success = *address;

// 	address = (char *)0x3FFFFF; // before kernel memory
// 	success = *address;

// 	address = (char *)0xC000; // after video memory
// 	success = *address;

// 	address = (char *)0x800000; // after kernel memory
// 	success = *address;

// 	return FAIL; // this line will only be reached if we are not able to 
// }

// PAGING TESTS COMPLETE

/*
 * int null_test();
 * Inputs: void
 * Return Value: FAIL if test case fails
 * Function: test case will fail if we somehow 
 */
// int null_test(){
// 	char success;
// 	char* address;
// 	address = (char *)0x0; // try to dereference null
// 	success = *address;
// 	return FAIL; // this line is only reached 
// }

// int out_of_bounds(){
// 	int val;
// 	int arr[3] = {0,0,0};
// 	val = arr[1000];
// 	return FAIL; // this line is only reached 
// }
 /*  					CHECKPOINT 1  TESTS  END              */


/* Checkpoint 2 tests */

/* TERMINAL DRIVER TESTS BEGIN */

/* 
 * test_term_read()
 *   DESCRIPTION: Test terminal read function
 *   INPUTS: none
 *   OUTPUTS: whatever is stored in the buffer
 *   RETURN VALUE: 1
 *   SIDE EFFECTS: none
 */
int test_term_read() {
	int32_t garbage;
	clear();
	char* buf;
	garbage = terminal_read(0, buf, 0);
	puts(buf);
	return 1;
}

/* 
 * test_term_r_and_w()
 *   DESCRIPTION: Test terminal read and terminal write functions
 *   INPUTS: none
 *   OUTPUTS: whatever is in the buffer
 *   RETURN VALUE: 1
 *   SIDE EFFECTS: none
 */
int test_term_r_and_w() {
	int32_t garbage;
	clear();
	char* buf;
	int num_bytes = terminal_read(0, buf, 0);
	cli();
	garbage = terminal_write(0, buf, num_bytes);
	sti();
	return 1;
}

/* 
 * test_term_write()
 *   DESCRIPTION: Test terminal write function
 *   INPUTS: none
 *   OUTPUTS: whatever is in the buffer, in this case "My name is luke"
 *   RETURN VALUE: 1
 *   SIDE EFFECTS: none
 */
int test_term_write() {
	clear();
	char* buf = "My name is luke";
	terminal_write(0, buf, strlen(buf));
	//putc('\n');
	return 1;
}

/* 
 * test_term_write_incorrect()
 *   DESCRIPTION: Test terminal write function but with invalid input
 *   INPUTS: none
 *   OUTPUTS: whatever is in the buffer, when size of buffer is exceeded
 *   RETURN VALUE: -1 if successful
 *   SIDE EFFECTS: none
 */
int test_term_write_incorrect() {
	clear();
	char* buf = "My name is luke ;alskdjf;alsdkjfa;lsdkjfa;lsdkasdjklsdfjkll;asdkasafasdfasdfasdfasdf"
				"a;lskdjf;alksdjf;alkjsdf;lkajsdf;lkjasdfkljas;ldksdfklja;slkdasdjklf;alksdjf;askldfj;sdlk";
	int ret = terminal_write(0, buf, strlen(buf));
	//putc('\n');
	return (ret == -1);
}

/* 
 * keyboard_test()
 *   DESCRIPTION: Test terminal read and terminal write functions with keyboard
 *   INPUTS: none
 *   OUTPUTS: whatever is in the buffer
 *   RETURN VALUE: 1 if successful
 *   SIDE EFFECTS: none
 */
int keyboard_test() {
	while (1) {
		char* buf;
		int num_bytes = terminal_read(0, buf, 0);
		//cli();
		terminal_write(0, buf, num_bytes);
		//sti();
	}
	return 1;
}

int keyboard_test_first_three() {
	while (1) {
		char buf[128];
		terminal_read(0, buf, 0);
		cli();
		terminal_write(0, buf, 3);
		sti();
	}
	return 1;
}

/* TERMINAL DRIVER TESTS END */


/* RTC TESTS BEGIN */

/* RTC TESTS END */


/* FILE SYSTEM DRIVER TESTS BEGIN */
/* 
 * list_files()
 *   DESCRIPTION: List Each Directory Entry and its Info; test directory read
 *   INPUTS: none
 *   OUTPUTS: the number of files, the file names, file sizes, and file types
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void list_files(){
	clear();			//clear screen
	printf("\n");
	printf(" \n");
	printf(" \n");
	uint32_t num_files = boot_block->dir_entries;  // get number of directory entries
	printf("Number of Files: %d\n", num_files);
	printf(" \n");
	int i;
	int j;
	inode_t * current_inode;
	for(i = 0; i < boot_block->dir_entries; i++){
		uint8_t buf[32];
		int32_t num_copied = directory_read(i, buf, num_files);			// read each file name from the directory and store in buff
		if(num_copied == -1){
			printf("Fail");
		}
		printf("File Name: ");
		for(j = 0; j < 32; j++){
 			printf("%c", buf[j]);   // print file name 
 		}
		printf(",   File Type: %d", boot_block->dentry_in_boot[i].file_type);  // print file type
		current_inode = (inode_t *)(in_memory_FS + ((boot_block->dentry_in_boot[i].inode_number + 1) * FILE_BLOCK_SIZE)); 
		printf(",   File Size: %d", current_inode->length);  // print file size 
		printf("\n");
	}
	
	return;
}

/* 
 * read_small_file_test()
 *   DESCRIPTION: Print the Contents of a Small Text File; test read_dentry_by_name
 *   INPUTS: none
 *   OUTPUTS: contents of the small text file
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void read_small_file_test(){
	clear();
	printf("\n");
	printf(" \n");
	printf(" \n");
	int32_t val;
	dentry_t text0_dentry;
	//val = read_dentry_by_index (10, &text0_dentry);
	const uint8_t* fname = (const uint8_t*) "frame0.txt";   // examine frame0.txt's contents
	val = read_dentry_by_name(fname, &text0_dentry);        // create a directory entry from file name
	if(val == -1){
		printf("Fail");
	}
	inode_t * current_inode;
	//current_inode = (inode_t *)(in_memory_FS + (boot_block->dentry_in_boot[10].inode_number + 1) * FILE_BLOCK_SIZE);
	current_inode = (inode_t *)(in_memory_FS + ((text0_dentry.inode_number + 1) * FILE_BLOCK_SIZE));  // get respective inode for file
	uint8_t buf[current_inode->length];							// get the length of the file
	val = file_read(10, buf, current_inode->length);			// 10 is the index of "frame0.txt" within the directory entries
																// read the contents of the file
	int i;
	for(i = 0; i < current_inode->length; i++){
		printf("%c", buf[i]);									// print contents of file
	}
	printf("\n");
	printf("Filename: %s", text0_dentry.fname);
	return;
}

/* 
 * read_large_file_test()
 *   DESCRIPTION: Print the Contents of a Large Text File; test read_dentry_by_name
 *   INPUTS: none
 *   OUTPUTS: contents of the large text file
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void read_large_file_test(){
	clear();
	printf("\n");
	printf(" \n");
	printf(" \n");
	int32_t val;
	dentry_t text0_dentry;
	//val = read_dentry_by_index (11, &text0_dentry); //11
	const uint8_t* fname = (const uint8_t*) "verylargetextwithverylongname.tx"; // examine "verylargetextwithverylongname.tx" contents
	val = read_dentry_by_name (fname, &text0_dentry); //11

	inode_t * current_inode;
	current_inode = (inode_t *)(in_memory_FS + (text0_dentry.inode_number + 1) * FILE_BLOCK_SIZE);
	uint8_t buf[current_inode->length];
	
	val = file_read(11, buf, current_inode->length); // 11 is the index of "verylargetextwithverylongname.tx" within the directory entries
	if(val == -1){
			printf("Fail");
		}
	int i;
	char first_words[] = "very large text file with a very long name";
	printf("Expected First Characters: %s\n", first_words);
	printf("Actual First Characters:   ");
	for(i = 0; i < strlen(first_words); i++){
		printf("%c", buf[i]);								// print first characters of buffer
	}
	printf("\n");
	printf("\n");
	char last_chars[] = "?~!@#$%^&*()_+`1234567890-=[]\\{}|;':\",./<>?";
	printf("Expected Last Characters: %s\n", last_chars);
	//printf("Last char: %c\n", buf[(current_inode->length)-2]);
	printf("Actual Last Characters:   ");
	for(i = current_inode->length - strlen(last_chars)-1; i < current_inode->length; i++){
		printf("%c", buf[i]);			// print last characters of buffer
	}
	printf("\n");
	int j;
	printf("Filename: ");
	for(j = 0; j < 32; j++){
 			putc(text0_dentry.fname[j]);
 		}
	return;
}

/* 
 * read_executible_file()
 *   DESCRIPTION: Print the Contents of a Executable File; test read_dentry_by_name
 *   INPUTS: none
 *   OUTPUTS: contents of the executable file
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void read_executible_file(){
	clear();
	printf("\n");
	printf(" \n");
	printf(" \n");
	int32_t val;
	dentry_t text0_dentry;
	//val = read_dentry_by_index (10, &text0_dentry);
	const uint8_t* fname = (const uint8_t*) "ls";				// examine ls's contents
	val = read_dentry_by_name (fname, &text0_dentry); //11
	if(val == -1){
		printf("Fail");
	}
	inode_t * current_inode;
	//current_inode = (inode_t *)(in_memory_FS + (boot_block->dentry_in_boot[10].inode_number + 1) * FILE_BLOCK_SIZE);
	current_inode = (inode_t *)(in_memory_FS + ((text0_dentry.inode_number + 1) * FILE_BLOCK_SIZE)); 
	uint8_t buf[current_inode->length];
	
	val = file_read(12, buf, current_inode->length); 			// 12 is the index of ls within the directory entries

	//val = read_data (text0_dentry.inode_number,0,buf,current_inode->length);
	int i;
	printf("First 20 Characters of Executable:\n");
	for(i = 0; i < 20; i++){
		printf("%c", buf[i]);					// print first 40 characters of exec
	}
	printf("\n");
	printf("Last 40 Characters of Executable:\n");
	for(i = current_inode->length - 40-1; i < current_inode->length; i++){
		printf("%c", buf[i]);					// print last 40 characters of exec
	}
	printf("\n");
	printf("\n");
	printf("Filename: %s", text0_dentry.fname);
}

/* 
 * open_bad_file_1()
 *   DESCRIPTION: Try to Open a Bad Text File; test file_open
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: FAIL if we return -1 from file_open; PASS if we don't
 *   SIDE EFFECTS: none
 */
int32_t open_bad_file_1(){
	int32_t ret_val;
	const uint8_t* fname = (const uint8_t*) "andrew.txt";   // file does not exist in directory
	ret_val = file_open(fname);
	if(ret_val == -1){
		return FAIL;
	}
	return PASS;
}

/* 
 * open_bad_file_2()
 *   DESCRIPTION: Try to Open a Bad Text File; test file_open
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: FAIL if we return -1 from file_open; PASS if we don't
 *   SIDE EFFECTS: none
 */
int32_t open_bad_file_2(){
	int32_t ret_val;
	const uint8_t* fname = (const uint8_t*) "verylargetextwithverylongname.txt"; // file is too long in size
	ret_val = file_open(fname);
	if(ret_val == -1){
		return FAIL;
	}
	return PASS;
}

/* 
 * write_file_test()
 *   DESCRIPTION: Write to a File; test file_write
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: FAIL if we return -1 from file_write; PASS if we don't
 *   SIDE EFFECTS: none
 */
int32_t write_file_test(){
	int32_t ret_val;
	uint32_t fd = 0;
	const uint8_t * buff;
	int32_t nbytes = 100;
	ret_val = file_write(fd, buff, nbytes);       // test write to file
	if(ret_val == -1){
		return FAIL;
	}
	return PASS;
}

/* 
 * close_file_test()
 *   DESCRIPTION: Close a File; test file_close
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: FAIL if we return -1 from file_write; PASS if we don't
 *   SIDE EFFECTS: none
 */
int32_t close_file_test(){
	int32_t ret_val;
	uint32_t fd = 1;
	ret_val = file_close(fd);       // test close file
	if(ret_val == -1){
		return FAIL;
	}
	return PASS;
}

/* 
 * open_bad_dir_test()
 *   DESCRIPTION: Try to open a bad directory name; test directory_open
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: FAIL if we return -1 from file_write; PASS if we don't
 *   SIDE EFFECTS: none
 */
int32_t open_bad_dir_test(){
	int32_t ret_val;
	const uint8_t* fname = (const uint8_t*) "test";
	ret_val = directory_open(fname);   // test opening bad directory name
	if(ret_val == -1){
		return FAIL;
	}
	return PASS;
}

// void fs_read_filename(){
// 	clear();
// 	printf("\n");
// 	printf(" \n");
// 	//boot_block->dentry_in_boot[i].fname
// 	dentry_t dir_entry; 
// 	printf("file name before: %s\n", boot_block->dentry_in_boot[12].fname);
// 	int32_t val = read_dentry_by_name (dir_entry.fname, &dir_entry);
// 	if (val == -1){
// 		printf("Fail");
// 	}

// 	printf("file name: %s", dir_entry.fname);
// }



/* FILE SYSTEM DRIVER TESTS END */


/* CHECKPOINT 3 TESTS START */ 

/* Checkpoint 3 tests */

int32_t bad_exec_name_1(){
	uint8_t command [] = "fish_house fish0.txt";
	int32_t ret  = sys_exec((const uint8_t*)command);
	if(ret == -1){
		return FAIL;
	}
	return PASS;

}

int32_t bad_exec_name_2(){
	uint8_t command [] = "frame0.txt";
	int32_t ret  = sys_exec((const uint8_t*)command);
	if(ret == -1){
		return FAIL;
	}
	return PASS;

}

void exec_test(){
	uint8_t command [] = "shell fish0.txt";
	int32_t ret  = sys_exec((const uint8_t*)command); 
	if(ret == -1){
		printf("Failed");
	}
}

void shell_test() {
	int32_t ret = sys_exec((const uint8_t*)"shell");
	if (ret == -1) {
		printf("Failed");
	}
}

void ls_test() {
	int32_t ret = sys_exec((const uint8_t*)"ls");
	if (ret == -1) {
		printf("Failed");
	}
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/* CHECKPOINT 1
	TEST_OUTPUT("idt_test", idt_test());
	TEST_OUTPUT("Divide by 0", div_by_0());
	TEST_OUTPUT("Dereference Kernel", dereferencing_kernel());
	TEST_OUTPUT("Dereference Video Memory", dereferencing_video_memory());
	TEST_OUTPUT("Dereference in between acceptable ranges", dereferencing_in_between());
	TEST_OUTPUT("Dereference in before and after acceptable ranges", dereferencing_before_and_after());
	TEST_OUTPUT("Null Test", null_test());
	CHECKPOINT 1 */

	/* CHECKPOINT 2*/
	//keyboard_test();
	//keyboard_test_first_three();

	//TEST_OUTPUT("Out of Bounds", out_of_bounds());
	//TEST_OUTPUT("RTC Read Test", test_rtc_read());
	/* FILE SYSTEM TESTS */

	//list_files();
	//read_small_file_test();
	//read_large_file_test();
	//fs_read_filename();
	//read_executible_file();
	//TEST_OUTPUT("Open Bad File 1", open_bad_file_1());
	//TEST_OUTPUT("Open Bad File 2", open_bad_file_2());
	//TEST_OUTPUT("Write File Test", write_file_test());
	//TEST_OUTPUT("Close File Test", close_file_test());
	//TEST_OUTPUT("Open Directory Test", open_bad_dir_test());

	/* CHECPOINT 3 */
	//TEST_OUTPUT("Open Bad Exec Command 1", bad_exec_name_1());
	//TEST_OUTPUT("Open Bad Exec Command 2", bad_exec_name_2());
	exec_test();
	// launch your tests here
}
