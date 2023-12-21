#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "interrupts.h"
#include "i8259.h"
#include "paging.h"
#include "devices/rtc.h"
#include "devices/keyboard.h"
#include "devices/terminal.h"
#include "devices/filesystem.h"
#include "system_calls.h"
#include "devices/pit.h"
#include "scheduler.h"

#define PASS 1
#define FAIL 0
#define COUNTER_LIMIT           1000000000
#define VIDEO_MEM_START         0xB8000
#define KERNEL_MEM_START        0x00400000
#define VIDEO_OUT_OF_BOUNDS     0xB9000
#define KERNEL_OUT_OF_BOUNDS    0x00800000
#define NUM_ROWS                25
#define RTC_BUF_SIZE            4
#define RTC_WRITE_SIZE          4
#define MAX_BUFFER_SIZE         128
#define COUNTER_DIVISOR         10
#define BYTES_4KB               4096
#define MAX_FILENAME_SIZE       32

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

// surround with if statement to prevent compile warnings
#if (RUN_TESTS)

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

	return result;
}


/* Divide by 0 Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 0
 * Files: interrupt.c/intr_asm_linkage.S
 */
int div_by_zero_test() {
    TEST_HEADER;
    asm volatile("int $0");
    return FAIL;
}

/* Debug Exception Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 1
 * Files: interrupt.c/intr_asm_linkage.S
 */
int debug_exception_test() {
    TEST_HEADER;
    asm volatile("int $1");
    return FAIL;
}

/* NMI Interrupt Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 2
 * Files: interrupt.c/intr_asm_linkage.S
 */
int nmi_interrupt_test() {
    TEST_HEADER;
    asm volatile("int $2");
    return FAIL;
}

/* Breakpoint Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 3
 * Files: interrupt.c/intr_asm_linkage.S
 */
int breakpoint_exception_test() {
    TEST_HEADER;
    asm volatile("int $3");
    return FAIL;
}

/* Overflow Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 4
 * Files: interrupt.c/intr_asm_linkage.S
 */
int overflow_exception_test() {
    TEST_HEADER;
    asm volatile("int $4");
    return FAIL;
}

/* Bound Range Exceeded Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 5
 * Files: interrupt.c/intr_asm_linkage.S
 */
int bound_range_exceeded_test() {
    TEST_HEADER;
    asm volatile("int $5");
    return FAIL;
}

/* Invalid Opcode Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 6
 * Files: interrupt.c/intr_asm_linkage.S
 */
int invalid_opcode_test() {
    TEST_HEADER;
    asm volatile("int $6");
    return FAIL;
}

/* Device Not Available Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 7
 * Files: interrupt.c/intr_asm_linkage.S
 */
int device_not_available_test() {
    TEST_HEADER;
    asm volatile("int $7");
    return FAIL;
}

/* Double Fault Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 8
 * Files: interrupt.c/intr_asm_linkage.S
 */
int double_fault_test() {
    TEST_HEADER;
    asm volatile("int $8");
    return FAIL;
}

/* Coproccesor Segment Overrun Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 9
 * Files: interrupt.c/intr_asm_linkage.S
 */
int coprocessor_segment_overrun_test() {
    TEST_HEADER;
    asm volatile("int $9");
    return FAIL;
}

/* Invalid TSS Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 10
 * Files: interrupt.c/intr_asm_linkage.S
 */
int invalid_tss_test() {
    TEST_HEADER;
    asm volatile("int $10");
    return FAIL;
}

/* Segment Not Present Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 11
 * Files: interrupt.c/intr_asm_linkage.S
 */
int segment_not_present_test() {
    TEST_HEADER;
    asm volatile("int $11");
    return FAIL;
}

/* Stack Fault Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 12
 * Files: interrupt.c/intr_asm_linkage.S
 */
int stack_fault_test() {
    TEST_HEADER;
    asm volatile("int $12");
    return FAIL;
}

/* General Protection Exception Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 13
 * Files: interrupt.c/intr_asm_linkage.S
 */
int general_protection_exception_test() {
    TEST_HEADER;
    asm volatile("int $13");
    return FAIL;
}

/* Page Fault Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 14
 * Files: interrupt.c/intr_asm_linkage.S
 */
int page_fault_test() {
    TEST_HEADER;
    asm volatile("int $14");
    return FAIL;
}

/* Assertion Error Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 15
 * Files: interrupt.c/intr_asm_linkage.S
 */
int assertion_error_test() {
    TEST_HEADER;
    asm volatile("int $15");
    return FAIL;
}

/* FPU Floating Point Error Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 16
 * Files: interrupt.c/intr_asm_linkage.S
 */
int fpu_floating_point_error_test() {
    TEST_HEADER;
    asm volatile("int $16");
    return FAIL;
}

/* Alignment Check Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 17
 * Files: interrupt.c/intr_asm_linkage.S
 */
int alignment_check_test() {
    TEST_HEADER;
    asm volatile("int $17");
    return FAIL;
}

/* Machine Check Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 18
 * Files: interrupt.c/intr_asm_linkage.S
 */
int machine_check_test() {
    TEST_HEADER;
    asm volatile("int $18");
    return FAIL;
}

/* SIMD Floating-Point Exception Test
 *
 * Checks that exception is shown on screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: IDT Exception 19
 * Files: interrupt.c/intr_asm_linkage.S
 */
int simd_floating_point_exception_test() {
    TEST_HEADER;
    asm volatile("int $19");
    return FAIL;
}


/* System Call Test
 *
 * Checks that system call prints to screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: System Call 0x80 in IDT
 * Files: interrupt.c/intr_asm_linkage.S
 */
int system_call_test(){
	TEST_HEADER;
	asm volatile("int $128");
	return PASS;
}

/* Check IRQ Disable
 *
 * Checks if IRQ Disable turns off interrupt on PIC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: irq_disable
 * Files: i8258.c
 */
int irq_disable_test() {
    TEST_HEADER;

    // allow typing for a couple of seconds
    int counter = 0;
    while(1) {
        counter++;
        if (counter >= COUNTER_LIMIT) {
            break;
        }
    }

    disable_irq(KEYBOARD_IRQ);
    return PASS;
}

/* RTC Test
 *
 * Checks if RTC works 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: rtc_init, rtc_handler
 * Files: interrupt.c
 */
int rtc_test() {
    TEST_HEADER;
    
    rtc_init();

    // allow rtc for a couple of seconds
    int counter = 0;
    while(1) {
        counter++;
        if (counter >= COUNTER_LIMIT) {
            break;
        }
    }

    disable_irq(RTC_IRQ);
    return PASS;
}

/* No Interrupt after Exception Test
 *
 * Checks if no interrupts can happen after exception is made
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception_handler
 * Files: interrupt.c
 */
int no_intr_after_exc_test() {
    TEST_HEADER;

    int counter = 0;
    while(1) {
        counter++;
        if (counter >= COUNTER_LIMIT) {
            break;
        }
    }

    // divide by 0 exception
    div_by_zero_test();
    return FAIL;
}



/* Validate Video Memory Page Test
 *
 * Checks if we can properly dereference existing pages in video mem
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: init_paging
 * Files: paging.c
 */
int deref_video_mem_test() {
    TEST_HEADER;

    uint8_t* a = (uint8_t*)VIDEO_MEM_START;
    uint16_t i;
    uint32_t b;
    for (i = 0; i < ALIGN_4KB; i++) {
        b = *(a+i);
    }
    b++;
    return PASS;
}

/* Validate Kernel Memory Page Test
 *
 * Checks if we can properly dereference page of kernel memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: init_paging
 * Files: paging.c
 */
int deref_kernel_mem_test() {
    TEST_HEADER;

    uint8_t* a = (uint8_t*)KERNEL_MEM_START;
    uint32_t i;
    uint32_t b;
    for (i = 0; i < ALIGN_4KB*PAGE_VEC; i++) {
        b = *(a+i);
    }
    b++;
    return PASS;
}

/* Non-Existing Page Dereference Test
 *
 * Checks if we can't dereference non-existing pages
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: init_paging
 * Files: paging.c
 */
int deref_non_existing_page() {
    TEST_HEADER;

    uint8_t* a = (uint8_t*) VIDEO_OUT_OF_BOUNDS;
    uint32_t b = *a;
    b++;
    return FAIL;
}

/* Non-Existing Page Directory Test
 *
 * Checks if we can't dereference non-existing directory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: init_paging
 * Files: paging.c
 */
int deref_non_existing_dir() {
    TEST_HEADER;

    uint8_t* a = (uint8_t*) KERNEL_OUT_OF_BOUNDS;
    uint32_t b = *a;
    b++;
    return FAIL;
}

/* Paging While Loop Test
 *
 * Inits Paging and stalls kernel
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: paging, interrupts
 * Files: kernel.c
 */
int paging_while_loop_test() {
    TEST_HEADER;

    while(1){};
    return PASS;
}

/* Dereferencing Null Test
 *
 * Checks if exception is thrown when dereferencing NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: exception_handler()
 * Files: interrupts.c
 */
int deref_null() {
    TEST_HEADER; 

    int* a = NULL;
    int b = *a;
    b++; 
    return FAIL;
}

/* Checkpoint 2 tests */

/* Video Memory Shift Test
 *
 * Prints text to the screen and shifts it upwards off the screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal
 * Files: terminal.c, keyboard.c
 */
int video_memory_shift_test() {
    int counter;
    int i = 0;
    // clear screen and set cursor to last line
    clear();
    set_cursor(0, 24);

    // print some text to the screen
    puts("Test Text");

    while(1) {
        if (i > NUM_ROWS) {
            break;
        }
        counter++;
        if (counter >= (COUNTER_LIMIT / COUNTER_DIVISOR)) {
            shift_video_mem_up(0, NUM_ROWS);
            i++;
            counter = 0;
        }
    }

    return PASS;
}

/* Terminal Test
 *
 * Opens terminal, starts a read, then executes writes
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_read, terminal_write
 * Files: terminal.c, keyboard.c
 */
int terminal_test() {
    char buf[MAX_BUFFER_SIZE];
    int num_chars;

    terminal_open(0);

    while (1) {
        num_chars = terminal_read(0, buf, 0);
        terminal_write(0, buf, num_chars);
    }

    return PASS;
}

/* Terminal Write String Test
 *
 * Opens terminal and executes writes of test strings
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_write
 * Files: terminal.c, keyboard.c
 */
int terminal_write_string_test() {
    char buf[MAX_BUFFER_SIZE];
    int num_chars;
    int i;
    char* test_string;

    terminal_open(0);

    test_string = "Terminal Write Test Command";
    num_chars = strlen(test_string);

    for (i = 0; i < num_chars; i++) {
        buf[i] = test_string[i];
    }

    // writes to the terminal NUM_ROWS times
    for (i = 0; i < NUM_ROWS; i++) {
        terminal_write(0, buf, num_chars);
    }

    terminal_close(0);

    return PASS;
}

/* Terminal Different Size Strings Test
 *
 * Opens terminal and executes writes of test strings of different lengths
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_open, terminal_write, terminal_close
 * Files: terminal.c, keyboard.c
 */
int terminal_diff_string_test() {
    char buf[MAX_BUFFER_SIZE + 2];
    int num_chars;
    int i;
    char* test_string;
    int ret_val;

    terminal_open(0);

    // writes a too long string to terminal
    test_string = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut en";
    num_chars = strlen(test_string);

    for (i = 0; i < num_chars; i++) {
        buf[i] = test_string[i];
    }

    ret_val = terminal_write(0, buf, num_chars);

    if (ret_val != 0) {
        return FAIL;
    }

    // num_chars > length of the string
    test_string = "Hello World!";
    num_chars = strlen(test_string) + 1;

    for (i = 0; i < num_chars; i++) {
        buf[i] = test_string[i];
    }

    ret_val = terminal_write(0, buf, num_chars);

    if (ret_val != strlen(test_string) + 1) { // need + 1 due to newline character
        return FAIL;
    }

    // num_chars < length of the string
    test_string = "Hello World!";
    num_chars = strlen(test_string) - 1;

    for (i = 0; i < num_chars; i++) {
        buf[i] = test_string[i];
    }

    ret_val = terminal_write(0, buf, num_chars);

    if (ret_val != num_chars) {
        return FAIL;
    }

    terminal_close(0);

    return PASS;
}

/* RTC Changing Frequency Test
 *
 * Writes different frequency to the RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: rtc_write, rtc_close, rtc_open, rtc_read
 * Files: rtc.c
 */
int test_rtc_cp2 () {

    uint32_t buf[1];
    long long i;
    int frequency = 2;  //starting rate of RTC test
    int ret_val;

    //open rtc tested when intializing in kernel

    while (frequency <= 128) {  // change to increase the number of values that are tested

        set_cursor(0, 0);
        printf("Current frequncy: %d", frequency);
        set_cursor(0, 2);


        buf[0] = frequency;
        //test write rtc
        ret_val = rtc_write(0, buf, RTC_BUF_SIZE);
        if (ret_val != RTC_WRITE_SIZE) {
            return FAIL;
        }
        
        i = 0;
        while (i <= COUNTER_LIMIT) {
            i++;
        }
        //test read rtc
        ret_val = rtc_read(0, buf, RTC_BUF_SIZE);
        if (ret_val != 0) {
            return FAIL;
        }
        frequency *= 2; //sets new frequency value by multiplying by 2
        clear();

    }
    //tests close rtc
    rtc_close(0);
    set_cursor(0, 0);
    return PASS;
}

/* RTC Write Test
 *
 * Writes frequency to the RTC
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: rtc_write
 * Files: rtc.c
 */
int rtc_write_test(int frequency) {
    int ret_val;
    ret_val = rtc_write(0, &frequency, RTC_BUF_SIZE);
    if (ret_val != RTC_WRITE_SIZE) {
        return FAIL;
    }
    return PASS;
}


/* Test Dentry Search
 *
 * validates filenames in directory entries are matching through both functions
 * read_dentry_by_index and read_dentry_by_name
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: read_dentry_by_index, read_dentry_by_name
 * Files: filesystem.c
 */
int32_t test_dentry_search(){
	TEST_HEADER;

	dentry_t access_by_name;
	dentry_t access_by_index;

	int index_bootblock = 3;
	read_dentry_by_index(index_bootblock, &access_by_index);
	if(read_dentry_by_name((uint8_t*)access_by_index.fileName, &access_by_name) == -1){return FAIL;}
	if (!(strncmp((char*)(access_by_index.fileName), (char*)(access_by_name.fileName), MAX_FILENAME_SIZE))){return PASS;}
	else{
		printf("Filenames don't match up");
		return FAIL;
	}
}

/* Test Read file
 *
 * reads specified file name and prints file to console
 * Inputs: string filename
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: file_open, file_read, read_dentry_by_name
 * Files: filesystem.c
 */
int test_file_read(char* filename) {
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	dentry_t test_dentry;
	int32_t ret_val = read_dentry_by_name((uint8_t*)filename, &test_dentry);	// inode = 10 =>frame0.txt (small file)
    if (ret_val == -1) {
        return FAIL;
    }
    file_open((uint8_t*)filename);
	uint32_t inode_index = test_dentry.inodeNumber;

	printf("FILENAME: %s \n", test_dentry.fileName);
	printf("INODE # of BYTES: %d \n", inode_start[inode_index].numBytes);
	int offset = 0;
	int32_t file_length = inode_start[inode_index].numBytes;
	char buff[file_length];																	// size of one block

    // 187 is file size according to inodes[test_inode_num].length
	int32_t num_bytes_copied = file_read(test_dentry.inodeNumber, offset, (uint8_t*)buff, file_length - offset);

	// print file
	if(num_bytes_copied == -1){return FAIL;}
	else{
		printf("%s \n", buff);
        if (file_length == num_bytes_copied + offset) {
            return PASS;
        }
		return FAIL;
	}
}

/* Test read file into a buffer
 *
 * reads specified file name and prints file to console
 * Inputs: filename - name of file to be read
 *         buf - buffer for data to be placed into
 *         nbytes - length of data to be read
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: file_open, file_read, read_dentry_by_name
 * Files: filesystem.c
 */
int test_buffer_file_read(char* filename, uint8_t* buf, uint32_t nbytes) {
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	dentry_t test_dentry;
    
	int32_t ret_val = read_dentry_by_name((uint8_t*)filename, &test_dentry);	// inode = 10 =>frame0.txt (small file)
    if (ret_val == -1) {
        return FAIL;
    }
    file_open((uint8_t*)filename);
	uint32_t inode_index = test_dentry.inodeNumber;

    // 187 is file size according to inodes[test_inode_num].length
	int32_t num_bytes_copied = file_read(inode_index, 0, buf, nbytes);

	// print file
	if(num_bytes_copied == -1) {
        return FAIL;
    }
	
    return PASS;

}

/* Test Write file
 *
 * Attempts to write to a specified file
 * Inputs: filename
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: file_write, read_dentry_by_name
 * Files: filesystem.c
 */
int test_file_write(char* filename) {
	clear();
	set_cursor(0, 0);
	TEST_HEADER;

	dentry_t test_dentry;
	int32_t ret_val = read_dentry_by_name((uint8_t*)filename, &test_dentry);
    if (ret_val == -1) {
        return FAIL;
    }
	uint32_t inode_index = test_dentry.inodeNumber;

	printf("FILENAME: %s \n", test_dentry.fileName);
	printf("INODE # of BYTES: %d \n", inode_start[inode_index].numBytes);
	int32_t file_length = inode_start[inode_index].numBytes;
	char buff[file_length];																	// size of one block
    buff[0] = 'A';

    // 187 is file size according to inodes[test_inode_num].length
	int32_t num_bytes_written = file_write(test_dentry.inodeNumber, (uint8_t*)buff, 1);

	if(num_bytes_written == -1) {
        return PASS;
    }
	
    return FAIL;
}

/* Test Read directory
 *
 * reads through the directory and prints filenames and types
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: directory_read, read_dentry_by_index
 * Files: filesystem.c
 */
int test_read_directory(){
	clear();
    set_cursor(0, 0);
	TEST_HEADER;
	char buf[33];
	uint32_t i = 0;
	while(directory_read(i, buf, MAX_FILENAME_SIZE) != 0) {
        dentry_t dentry_val;
        read_dentry_by_index(i, &dentry_val);
        printf("File Name: ");
		printf("%s", buf);
        printf(", file_type: ");
        printf("%d", dentry_val.fileType);
        printf("  file_size: ");
        printf("%u", (uint32_t)((inode_start[((bootblock->bootDentries)[i].inodeNumber)]).numBytes));
		printf("\n");
        i++;
	}
	return PASS;
}


/* Test Write directory
 *
 * Attempts to write to the directory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: directory_write
 * Files: filesystem.c
 */
int test_write_directory(){
	clear();
	TEST_HEADER;
	char buf[33];
    buf[0] = 'A';
    if (directory_write(0, buf, 1) == -1) {
        return PASS;
    }
	return FAIL;
}

/* Checkpoint 3 tests */

/* Test Open/Close file
 *
 * Opens two files, then closes the first one, then opens a new file
 * Inputs: filename_1 -- first file to open
 *         filename_2 -- second file to open
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: open, close, file_open, file_close, sys_call_linkage
 * Files: sys_call_asm_linkage.S, system_calls.c, filesystem.c
 */
int test_syscall_open_close(char* filename_1, char* filename_2){
    int ret_val;
    int32_t file_1_fd;

    init_current_pcb();

    // open file 1
	asm volatile (
        "movl %1, %%ebx;"    // Load filename_1 into EBX
        "movl $5, %%eax;"    // Load open number into EAX
        "int $0x80;"         // Call interrupt 0x80
        "movl %%eax, %0;"    // Move the return value from EAX to ret_val
        : "=r" (ret_val)
        : "r" (filename_1)
        : "%ebx", "%eax"
    );

    if (ret_val != 2) {
        return FAIL;
    }

    file_1_fd = ret_val;

    // open file 2
    asm volatile (
        "movl %1, %%ebx;"    // Load filename_2 into EBX
        "movl $5, %%eax;"    // Load open number into EAX
        "int $0x80;"         // Call interrupt 0x80
        "movl %%eax, %0;"    // Move the return value from EAX to ret_val
        : "=r" (ret_val)
        : "r" (filename_2)
        : "%ebx", "%eax"
    );

    if (ret_val != 3) {
        return FAIL;
    }

    // close file 1
    asm volatile (
        "movl %1, %%ebx;"    // Load file_1_fd into EBX
        "movl $6, %%eax;"    // Load close number into EAX
        "int $0x80;"         // Call interrupt 0x80
        "movl %%eax, %0;"    // Move the return value from EAX to ret_val
        : "=r" (ret_val)
        : "r" (file_1_fd)
        : "%ebx", "%eax"
    );

    if (ret_val != 0) {
        return FAIL;
    }

    // open file 1
	asm volatile (
        "movl %1, %%ebx;"    // Load filename_1 into EBX
        "movl $5, %%eax;"    // Load open number into EAX
        "int $0x80;"         // Call interrupt 0x80
        "movl %%eax, %0;"    // Move the return value from EAX to ret_val
        : "=r" (ret_val)
        : "r" (filename_1)
        : "%ebx", "%eax"
    );

    if (ret_val != 2) {
        return FAIL;
    }

    return PASS;
}

/* Test Execute
 *
 * Opens two files
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: open, file_open, sys_call_linkage
 * Files: sys_call_asm_linkage.S, system_calls.c
 */
int test_syscall_execute(char* filename){
    int ret_val;

    terminal_open((const uint8_t*)"test");

	asm volatile (
        "movl %1, %%ebx;"    // Load filename_1 into EBX
        "movl $2, %%eax;"    // Load 5 into EAX
        "int $0x80;"         // Call interrupt 0x80
        "movl %%eax, %0;"    // Move the return value from EAX to ret_val
        : "=r" (ret_val)
        : "r" (filename)
        : "%ebx", "%eax"
    );

    if (ret_val == -1) {
        return FAIL;
    }

    return PASS;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* PIT Test
 *
 * Checks if PIT works 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: pit_init
 * Files: interrupt.c
 */
int pit_test() {
    
    init_pit();

    // allow pit for a couple of seconds
    int counter = 0;
    while(1) {
        counter++;
        if (counter >= COUNTER_LIMIT) {
            break;
        }
    }

    disable_irq(PIT_LINE);
    return PASS;
}


/* Test suite entry point */
void launch_tests(){

    // uint8_t buf[32];
    // uint32_t nbytes = 32;
	
/*---------------------------------------------GENERAL CP1 CHECKS-----------------------------------------------------------*/
    // TEST_OUTPUT("idt_test", idt_test());
    // TEST_OUTPUT("deref_video_mem_test", deref_video_mem_test());
    // TEST_OUTPUT("deref_kernel_mem_test", deref_kernel_mem_test());
	// TEST_OUTPUT("system_call_test", system_call_test());

/*--------------------------------------------CP1 DEMO TESTS----------------------------------------------------------------*/
    // TEST_OUTPUT("irq_disable_test", irq_disable_test());
    // TEST_OUTPUT("rtc_test", rtc_test());

    // TEST_OUTPUT("no_intr_after_exc_test", no_intr_after_exc_test());
	// TEST_OUTPUT("div_by_zero_test", div_by_zero_test());

    // TEST_OUTPUT("deref_non_existing_page", deref_non_existing_page());
    // TEST_OUTPUT("deref_non_existing_dir", deref_non_existing_dir());

    // TEST_OUTPUT("paging_while_loop_test", paging_while_loop_test());
    // TEST_OUTPUT("deref_null", deref_null());

/*---------------------------------------------GENERAL CP2 TESTS------------------------------------------------------------*/

    // TEST_OUTPUT("video memory shift test", video_memory_shift_test());
    // TEST_OUTPUT("terminal write string test", terminal_write_string_test());
    // TEST_OUTPUT("dentry_search test", test_dentry_search());
    // TEST_OUTPUT("terminal diff size strings test", terminal_diff_string_test());

/*--------------------------------------------CP2 DEMO TESTS----------------------------------------------------------------*/

    // TEST_OUTPUT("terminal test", terminal_test());
    // TEST_OUTPUT("read_data test", test_file_read("frame0.txt"));
    // TEST_OUTPUT("test file read with buffer", test_buffer_file_read("frame0.txt", buf, nbytes));
    // TEST_OUTPUT("write data test", test_file_write("frame0.txt"));
    // TEST_OUTPUT("read_directory test", test_read_directory());
    // TEST_OUTPUT("write_directory test", test_write_directory());
    // TEST_OUTPUT("Sets a frequency", rtc_write_test(128));
    // TEST_OUTPUT("RTC Clock Test: ", test_rtc_cp2()); // Need to set RTC_TEST to 1 in tests.h

/*---------------------------------------------GENERAL CP3 TESTS------------------------------------------------------------*/



/*--------------------------------------------CP3 DEMO TESTS----------------------------------------------------------------*/

    // TEST_OUTPUT("syscall open/close test", test_syscall_open_close("frame0.txt", "verylargetextwithverylongname.tx"));
    // TEST_OUTPUT("syscall execute test", test_syscall_execute("shell"));

/*---------------------------------------------GENERAL CP5 TESTS------------------------------------------------------------*/

    // TEST_OUTPUT("PIT TEST", pit_test());


/*------------------------------------------ALL EXCEPTION TESTS-------------------------------------------------------------*/  
	// TEST_OUTPUT("div_by_zero_test", div_by_zero_test());
	// TEST_OUTPUT("debug_exception_test", debug_exception_test());
	// TEST_OUTPUT("nmi_interrupt_test", nmi_interrupt_test());
	// TEST_OUTPUT("breakpoint_exception_test", breakpoint_exception_test());
	// TEST_OUTPUT("overflow_exception_test", overflow_exception_test());
	// TEST_OUTPUT("bound_range_exceeded_test", bound_range_exceeded_test());
	// TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	// TEST_OUTPUT("device_not_available_test", device_not_available_test());
	// TEST_OUTPUT("double_fault_test", double_fault_test());
	// TEST_OUTPUT("coprocessor_segment_overrun_test", coprocessor_segment_overrun_test());
	// TEST_OUTPUT("invalid_tss_test", invalid_tss_test());
	// TEST_OUTPUT("segment_not_present_test", segment_not_present_test());
	// TEST_OUTPUT("stack_fault_test", stack_fault_test());
	// TEST_OUTPUT("general_protection_exception_test", general_protection_exception_test());
	// TEST_OUTPUT("page_fault_test", page_fault_test());
	// TEST_OUTPUT("assertion_error_test", assertion_error_test());
	// TEST_OUTPUT("fpu_floating_point_error_test", fpu_floating_point_error_test());
	// TEST_OUTPUT("alignment_check_test", alignment_check_test());
	// TEST_OUTPUT("machine_check_test", machine_check_test());
	// TEST_OUTPUT("simd_floating_point_exception_test", simd_floating_point_exception_test());
}

#endif
