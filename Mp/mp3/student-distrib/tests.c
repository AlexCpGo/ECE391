#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "rtc.h"
#include "fs.h"
#include "terminal.h"

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
 * Asserts that first 20 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	if ((idt[30].offset_15_00 == NULL) && 
		(idt[30].offset_31_16 == NULL)){
		assertion_failure();
		result = FAIL;				
	}
	
	return result;
}

// add more tests here
/* Check exception with zero as dividend */
int Divide_by_zero(){
	TEST_HEADER;
	int result, a, b;
	a = 10;
	b = 0;
	result = a/b;
	return result;
}

/* Test for enable op and disable op of irq for PIC */
void test_i8259_enable_irq(){
	/*Enable the interrupt from rtc*/
	enable_irq(PIC_irq_num);
	printf("i8259_MASTER_DATA is %d\n",i8259_MASTER_DATA);
}

void test_i8259_disable_irq(){
	/*send irq_num of keyboard and rtc and check whether it is masked*/
	disable_irq(PIC_irq_num);
}

/* Test for garbage */
void test_i8259_disable_irq_garbage(){
	/*Send port number which is larger than 15 to PIC*/
	disable_irq(100);
}
void test_i8259_enable_irq_garbage(){
	/*Send port number which is larger than 15 to PIC*/
	enable_irq(50);
}

/* Test for paging */
int paging_test() {
	TEST_HEADER;
	int result = PASS;

	// check dereferencing
	uint32_t *ptr = (uint32_t*) 0x400000;
	printf("Check kernel addr %x...", ptr);
	uint32_t val = *ptr;
	printf("Value at %x is %x\n", ptr, val);

	ptr = (uint32_t*)0xB8000;
	printf("Check video mem addr %x...", ptr);
	val = *ptr;
	printf("Value at %x is %x\n", ptr, val);

	return result;
}

int check_invalid_addr() {
	TEST_HEADER;

	uint32_t *ptr = (uint32_t*)0x200000;
	printf("Check kernel addr %x...", ptr);
	uint32_t val = *ptr;
	printf("Value at %x is %x\n", ptr, val);
	
	return FAIL;
}

/* Checkpoint 2 tests */


/* rtc_test
 * 
 *  Description: Tries out all possible frequencies for RTC and prints 1 according to frequency
 *  Inputs: None
 *  Outputs: PASS
 *  Side Effects: Prints 13 lines of 10 1s onto the screen
 *  Coverage: RTC drivers
 *  Files: rtc.h/c
 */
int rtc_test() {
    unsigned char freq[1];
	int char_count;
	int i;
	uint8_t * temp_open  = (uint8_t *) 0;
	TEST_HEADER;

    rtc_open(temp_open);

	
	int char_bound = 8; //Initially print out six chars of "1" 
	// Loop to try out all frequencies
	for(i = 0; i < 13; i++){
		freq[0] = 15 - i;
		char_count = 0; 
		// loop to print '1' based on the frequency
		while(char_count < char_bound){
			putc('1');
			// temporary use freq as the buf and sizeof for rtc_read since no full implementation yet
			rtc_read(0,freq, sizeof(freq));
			char_count++;
		}

		// print new line after finish printing for 1 frequency
		putc('\n');
		
		//As the freq of rtc increases, the screen update more faster, thus putc more "1" to recognize it
		char_bound = char_bound + 2*i + 1; // A random formula to increase the bound for char to print.
		rtc_write(0, freq, sizeof(freq));
	}
	rtc_close(0);
	return PASS;
}

/* ls_test
 *
 *  DESCRIPTION: this function is to show the list of current directory
 *  INPUT : none
 *  OUTPUT : PASS/FAIL
 *  SIDE EFFECTS : check wehter we can open the directory, and show the list name of the current directory
*/

int ls_test() {
	clear();
	TEST_HEADER;

	// open dir
	uint8_t* dir = (uint8_t*)".";
	if (dir_open(dir) != 0) {
		printf("Failed to open root dir\n");
		assertion_failure();
		return FAIL;
	}
	printf("--------------------Opened root dir--------------------\n");

	// ls
	uint8_t fname[FILENAME_LEN+1];
	printf("Filname\tType\n");
	while (1) {
		int len = dir_read(0, fname, FILENAME_LEN);
		if (len == -1) {
			printf("Failed to read dir\n");
			assertion_failure();
			return FAIL;
		}
		if (len == 0) {
			break;
		}
		fname[len] = '\0';

		dentry_t dentry;
		if (read_dentry_by_name(fname, &dentry)) {
			printf("Failed to read dir info\n");
			assertion_failure();
			return FAIL;
		}
		printf("%s %d\n", fname, dentry.file_type);
	}
	printf("--------------\n");

	if (dir_close(0) != 0) {
		printf("Failed to close root dir\n");
		assertion_failure();
		return FAIL;
	}
	printf("--------------------Closed root dir--------------------\n");

	return PASS;
}

/* display_file_content
 *
 *  DESCRIPTION: this function is to display the file content
 *  INPUT: fname
 *  OUTPUT: PASS/FAIL
 *  SIDE EFFECTS: check whether we can display the file content, and show content of file to the screen
 */
int display_file_content(const char* fname) {
	dentry_t dentry;
	if (read_dentry_by_name(fname, &dentry) != 0) {
		printf("File %s does not exist\n", fname);
		return FAIL;
	}

	const uint32_t len = 1024; //lengh of the buffer
	uint32_t offset = 0;
	uint8_t buf[len+1];
	int32_t read_len;
	do {
		int i;
		read_len = read_data(dentry.inode_idx, offset, buf, len);
		if (read_len < 0) {
			printf("Failed to read file %s\n", fname);
			return FAIL;
		}
		for (i = 0; i < read_len; i++)
			putc(buf[i]);
		offset += read_len;
	} while (len == read_len);
	putc('\n');

	return PASS;
}

/* cat_test
 *
 *  DESCRIPTION: this function is to do the cat test and call the function to show the content of file
 *  INPUT: none
 *  OUTPUT: PASS/FAIL
 *  SIDE EFFECTS: show the result of terminal read and display the file content
 */
int cat_test() {
	clear();
	TEST_HEADER;
	
	// test open
	if (open_terminal(0) != 0) {
		printf("Failed to open terminal\n");
		assertion_failure();
		return FAIL;
	}
	printf("--------------------Opened terminal--------------------\n");

	// test cat
	while (1) {
		int len;
	    char buffer[BUFFER_SIZE];
		printf("Enter a filename: (enter 'q' to quit)\n");

		read_terminal(0, buffer, BUFFER_SIZE);
		len = counter;
		
		
		if (len < 0) {
			printf("Failed to read from terminal\n");
			assertion_failure();
			return FAIL;
		}
		
		//printf("Buffer is %c",*buffer);
		if (strncmp(buffer, "q", BUFFER_SIZE) == 0) {
			printf("bye\n");
			break;
		}

		buffer[len-1] = '\0';
		display_file_content(buffer);
	}

	// test close
	if (close_terminal(0) != 0) {
		printf("Failed to close terminal\n");
		assertion_failure();
		return FAIL;
	}
	printf("--------------------Closed Terminal--------------------\n");
	return PASS;
}


/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	/*------------------------------------Test for Checkpoint 3.1---------------------------------------------*/
	//printf("\n");
	//printf("\n");
	//printf("\n");
	//printf("----------------------Test for IDT------------------------\n");
	//TEST_OUTPUT("idt_test", idt_test()); 
	//printf("--------------------Test for Exception-----------------------\n");
	//TEST_OUTPUT("Divide_by_zero_test",Divide_by_zero());
	//printf("\n");
	//printf("------------------Test for GarbageToPIC -------------------\n");
	//test_i8259_disable_irq_garbage();
	//test_i8259_enable_irq_garbage();
	//printf("---------Avaliableenable/disable---------------\n:");
	//test_i8259_enable_irq();
	//test_i8259_disable_irq();
	//printf("--------------------Test for Keyboard---------------------\n:");
	//printf("You can put the keyboard to enter your input\n");
	// printf("/*--------------------Test for RTC INTERRUPT-------------------*/\n");
	// rtc_interrupt();
	// printf("rtc_interrupt succeed\n");
	//clear();
	//printf("---------------------Test for Paging--------------------------\n");
	//TEST_OUTPUT("Paging_test",paging_test());
	//printf("\n");
	//clear();
	// TEST_OUTPUT("Invalid_addr_test",check_invalid_addr());
	

	/*------------------------------------Test for Checkpoint 3.2---------------------------------------------*/
	printf("====================TEST FOR RTC BEGINS============================\n"); 
	TEST_OUTPUT("rtc_test",rtc_test());
	printf("====================TEST FOR RTC ENDS==============================\n\n\n");
	
	printf("=================TESTS FOR FILE SYSTEM BEGIN=======================\n");
	// Able to read small files (frame0.txt, frame1.txt), executables (grep, ls) and large files (fish, verylargetextwithverylongname.tx(t)).
	TEST_OUTPUT("cat_test", cat_test());
	
	// list out all files of the root dir 
	TEST_OUTPUT("ls_test", ls_test());
	printf("=================TESTS FOR FILE SYSTEM END=========================\n");
	
	
	
}
