#include "system_calls.h"
#include "devices/terminal.h"
#include "devices/rtc.h"
#include "devices/filesystem.h"
#include "paging.h"
#include "x86_desc.h"
#include "interrupts.h"
#include "scheduler.h"

#define EXEC_BUF_LEN 1026
#define MAX_EXEC_ARG_LEN 1023
#define HEADER_LEN 40
#define MAGIC_NUM_LEN 4
#define ENTRY_PT_START 24
#define ENTRY_PT_LEN 4
#define SIZE_OF_BYTE 8
#define LOW_BYTE_MASK 0xFF
#define ADDR_128MB 0x08000000
#define PROG_IMG_OFFSET 0x00048000
#define FOUR_MB            0x400000
#define NUM_PIDS 6
#define STDIN_FD 0
#define STDOUT_FD 1
#define EIGHT_KB           0x2000
#define FOUR_KB            0x1000
#define EIGHT_MB           0x800000
#define EXCEPTION_RET_VAL       256
#define MAX_FILENAME_LEN        32

uint8_t file_check[MAGIC_NUM_LEN] = {0x7f, 0x45, 0x4c, 0x46}; // magic numbers to check if file is executable

struct term_table_t terminal_op_table = {terminal_open, terminal_read, terminal_write, terminal_close};
struct file_table_t file_op_table = {file_open, file_read, file_write, file_close};
struct rtc_table_t rtc_op_table = {rtc_open, rtc_read, rtc_write, rtc_close};
struct dir_table_t dir_op_table = {directory_open, directory_read, directory_write, directory_close};


/* int32_t halt(uint8_t status)
 * 
 * Returns from a process to a previous user process.
 * 
 * Inputs: status -- return value of process
 * Return Value: -1 on failure, 256 if process exits with exception, 0-255 if process exits properly
 * Function: halts program
 */
int32_t halt(uint8_t status)
{
    int i;
    uint32_t ebp_temp_val, esp_temp_val, ret_val, temp_pid;

    // get cached values
    ebp_temp_val = current_pcb->ebp_val;
    esp_temp_val = current_pcb->esp_val;

    // handle exception in child process
    if (exception_in_child) {
        ret_val = EXCEPTION_RET_VAL;
        exception_in_child = 0;
    } else {
        ret_val = (uint32_t)status;
    }

    // close current open file descriptors
    for (i = 0; i < FD_ARRAY_LENGTH; i++)
    {
        close(i);
    }

    cli();
    // empty stored EBP and ESP values
    current_pcb->ebp_val = 0;
    current_pcb->esp_val = 0;

    // allow process_id to be used
    current_pcb->in_use = 0;

    // handle base shell case
    if (current_pcb->process_id < MAX_TERMINALS)
    {
        temp_pid = current_pcb->process_id;

        current_pcb->parent_process_id = 0;
        current_pcb->process_id = 0;

        current_pcb = 0;

        tss.esp0 = EIGHT_MB - (EIGHT_KB * temp_pid) - sizeof(int);
        sti();
        execute((const uint8_t*)"shell");
    }

    // find parent pcb
    pcb_t* parent_pcb = get_pcb_ptr(current_pcb->parent_process_id);

    tss.esp0 = EIGHT_MB - (EIGHT_KB * current_pcb->parent_process_id) - sizeof(int);

    current_pcb->parent_process_id = 0;
    current_pcb->process_id = 0;

    // update current page
    directoryArray[USER_SPACE_DIR_NUM].offset_31_12 = get_phys_addr(parent_pcb->process_id);
    flush_tlb();

    current_pcb = parent_pcb;

    terminals[scheduled_terminal].active_pid = current_pcb->process_id;
    sti();

    // return to parent
    asm volatile(
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "xorl %%eax, %%eax;"
        "movl %2, %%eax;"
        "leave;"
        "ret;"
        :
        : "r"(ebp_temp_val), "r"(esp_temp_val), "r"(ret_val)
        : "cc", "%eax", "%ebp", "%esp");

    return -1;
}


/* int32_t execute(const uint8_t* command)
 * 
 * This function is called when a new program should be run. It sets up paging for a file,
 * creates the PCB and assigns process ID, and context switches.
 * 
 * Inputs: command -- string containing command to execute
 * Return Value: -1 on failure, 256 if process exits with exception, 0-255 if process exits properly
 * Function: executes program
 */
int32_t execute (const uint8_t* command) {
    uint8_t arg[EXEC_ARG_LEN];
    uint8_t filename[MAX_FILENAME_LEN];
    int i;
    uint32_t process_id;
    uint8_t header[ENTRY_PT_START + ENTRY_PT_LEN];
    dentry_t file_dentry;
    uint32_t entry_point;
    int cmd_len;
    int arg_start_idx = 0;
    int arg_index = 0;

    /*Parsing Args*/
    cmd_len = strlen((const int8_t*) command);

    //checks to see if command is valid, returns -1 if too long or null
    if (EXEC_ARG_LEN < cmd_len || command == NULL) {
        return -1;
    }

    //resets the arg array
    for(i = 0; i < EXEC_ARG_LEN; i++){
        arg[i] = '\0';
    }

    //resets the filename array
    for(i = 0; i < MAX_FILENAME_LEN; i++) {
        filename[i] = '\0';
    }

    //goes through command and sets where the args starts while filling in the filename array
    for(i = 0; i < cmd_len; i++) {
        if (command[i] == ' ') {
            arg_start_idx = i + 1;
            break;
        } else {
            filename[i] = command[i];
        }
    }

    //starts at the where the args start and fills in the arg index
    if (arg_start_idx) {
        for(i = arg_start_idx; i < cmd_len; i++) {
            arg[arg_index] = command[i];
            arg_index++;
        }
    }

    /*Check file validity*/

    // file doesn't exist
    if (read_dentry_by_name(filename, &file_dentry) != 0)
    {
        return -1;
    }

    // get file header
    read_data(file_dentry.inodeNumber, 0, header, (ENTRY_PT_START + ENTRY_PT_LEN));

    // check first 4 bytes
    for (i = 0; i < MAGIC_NUM_LEN; i++)
    {
        if ((uint8_t)header[i] != file_check[i])
        {
            return -1;
        }
    }

    // get entry point
    entry_point = 0;
    for (i = 0; i < ENTRY_PT_LEN; i++)
    {
        entry_point |= ((header[i + ENTRY_PT_START] & LOW_BYTE_MASK) << (i * SIZE_OF_BYTE));
    }

    // check if we're swapping temrinals for the first time
    if (!initialized_terminals[active_terminal]) {
        process_id = active_terminal;
    } else {
        // assign process_id
        process_id = get_pid();
    }

    // no process ids available
    if (process_id == -1) {
        return -1;
    }

    /* Set up Paging
     1. create 4mb physical page in directory
     2. copy program data to that page
     3. link that page to appropriate address in virtual memory
     can swap 2 and 3
    */
   
    // create page in page directory

    // USER_SPACE_DIR_NUM = 128MB / 4MB page size = 32

    directoryArray[USER_SPACE_DIR_NUM].offset_31_12 = get_phys_addr(process_id);

    flush_tlb();
    // copy program data to 4MB space in directory
    read_data(file_dentry.inodeNumber, 0, (uint8_t *)(ADDR_128MB + PROG_IMG_OFFSET), FOUR_MB);

    

    /* create pcb */

    pcb_t* new_pcb = get_pcb_ptr(process_id);

    new_pcb->in_use = 1;

    for (i = 0; i < FD_ARRAY_LENGTH; i++)
    {
        fd_element_t empty = {.file_op_table_ptr = 0, .file_position = 0, .flags = 0, .inode_num = 0};
        new_pcb->fd_array[i] = empty;
    }

    new_pcb->process_id = process_id;
    new_pcb->ebp_val = 0;
    new_pcb->esp_val = 0;

    // initial case, open 3 shells with different terminals, set active terminal to 0
    if (process_id < MAX_TERMINALS) {
        if (process_id == 0) {
            active_terminal = TERMINAL_1;
            // reserving pids 1 & 2
            pcb_t* temp_pcb = get_pcb_ptr(TERMINAL_2);
            temp_pcb->in_use = 1;
            temp_pcb = get_pcb_ptr(TERMINAL_3);
            temp_pcb->in_use = 1;
        }
        new_pcb->parent_process_id = -1;
    } else {
        new_pcb->parent_process_id = terminals[active_terminal].active_pid;
    }

    if (arg_start_idx < cmd_len) {
        strcpy((int8_t*)(new_pcb->arg), (int8_t*)arg);
    }

    /* add stdin, stdout to fda */

    new_pcb->fd_array[STDIN_FD].file_op_table_ptr  = (int32_t *)&terminal_op_table;
    new_pcb->fd_array[STDIN_FD].flags = 1;

    new_pcb->fd_array[STDOUT_FD].file_op_table_ptr  = (int32_t *)&terminal_op_table;
    new_pcb->fd_array[STDOUT_FD].flags = 1;

    // save for halt return
    tss.esp0 = (EIGHT_MB - (EIGHT_KB * (process_id))) - sizeof(int);

    register uint32_t stored_esp asm("esp");
    register uint32_t stored_ebp asm("ebp");

    new_pcb->ebp_val = stored_ebp;
    new_pcb->esp_val = stored_esp;

    /* context switch */
    current_pcb = new_pcb;

    // might need to be scheduled_terminal
    terminals[active_terminal].active_pid = current_pcb->process_id;

    // activate terminal so scheduler can now move to this terminal
    if (!initialized_terminals[active_terminal]) {
        initialized_terminals[active_terminal] = 1;
        
        // wait for pit interrupt to set scheduled_terminal to active terminal
        // that way, when iret-ing to shell user code, we are set to the correct terminal
        while(1) {
            if (active_terminal == scheduled_terminal) {
                break;
            }
        }
    }

    // currently, we go to the new program we just created, but then we start executing it out of order
    asm volatile(
        "pushl %0;" // push USER_DS
        "pushl %1;" // push ESP
        "pushfl;"   // push EFLAG
        "pushl %2;" // push USER_CS
        "pushl %3;" // push EIP
        "iret;"
        :
        : "r"(USER_DS), "r"(USER_ESP), "r"(USER_CS), "r"(entry_point)
        : "memory");

    return -1;
}

/* int32_t read(const int32_t fd, void* buf, const int32_t nbytes)
 *
 * This function reads nbytes bytes from the file with file descriptor fd and stores them in buf.
 *
 * Inputs: fd - file descriptor for fda, buf - buffer for reading bytes, nbytes - nbytes to read
 * Return Value: number of bytes read on success, -1 on failure
 * Function: calls appropriate read function based on file desc
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    dentry_t file_dentry;
    int32_t dentry_index;

    // check index
    if (fd < 0 || fd >= FD_ARRAY_LENGTH)
    {
        return -1;
    }

    // file desc isn't active
    if (current_pcb->fd_array[fd].flags != 1)
    {
        return -1;
    }

    // check for stdout
    if (fd == 1)
    {
        return -1;
    }

    // call read function from fd array for terminal
    if (fd == 0)
    {
        int32_t (*read)(int32_t, void *, int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[1];
        int32_t ret_val = (*read)(fd, buf, nbytes);
        return ret_val;
    }

    if (-1 == (dentry_index = find_dentry_by_inode_num(current_pcb->fd_array[fd].inode_num))) {
        return -1;
    }

    if (read_dentry_by_index(dentry_index, &file_dentry) != 0)
    {
        return -1; // fail condition for if can't read dentry by inode
    }

    // call rtc read
    if (file_dentry.fileType == RTC_FILE_TYPE)
    {
        int32_t (*read)(int32_t, void *, int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[1];
        int32_t ret_val = (*read)(fd, buf, nbytes);
        return ret_val;
        // call dir read
    }
    else if (file_dentry.fileType == DIRECTORY_FILE_TYPE)
    {
        int32_t (*read)(int32_t, void *, int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[1];
        int32_t ret_val = (*read)(current_pcb->fd_array[fd].file_position, buf, nbytes);
        current_pcb->fd_array[fd].file_position += 1;
        return ret_val;
        // call file read
    }
    else
    {
        int32_t (*read)(uint32_t, int32_t, void *, int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[1];
        int32_t ret_val = (*read)(current_pcb->fd_array[fd].inode_num, current_pcb->fd_array[fd].file_position, buf, nbytes);
        current_pcb->fd_array[fd].file_position += ret_val;
        return ret_val;
    }
}

/* int32_t write(int32_t fd, const void* buf, int32_t nbytes)
 *
 * This function writes nbytes bytes from buf to the file with file descriptor fd.
 *
 * Inputs: fd - file desc, buf - string to write, nbytes - number of bytes to write
 * Return Value: number of bytes written on success, -1 on failure
 * Function: calls appropriate write function based on file desc
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    // check index
    if (fd < 0 || fd >= FD_ARRAY_LENGTH)
    {
        return -1;
    }

    // file desc isn't active
    if (current_pcb->fd_array[fd].flags != 1)
    {
        return -1;
    }

    // check for stdin
    if (fd == 0)
    {
        return -1;
    }

    int32_t (*write)(int32_t, const void *, int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[2];

    // call write function from fd array for terminal
    if (fd == 1)
    {
        int32_t ret_val = (*write)(fd, buf, nbytes);
        return ret_val;
    }

    if ((rtc_table_t*) current_pcb->fd_array[fd].file_op_table_ptr == &rtc_op_table)
    {
        int32_t ret_val = (*write)(fd, buf, nbytes);
        return ret_val;
    }
    else
    {
        return -1; // read only file system
    }
}

/* int32_t open(const uint8_t* filename)
 * Inputs:      filename - filename to open
 * Return Value: 0 on success, -1 on failure
 * Function: opens file and initializes FD array element */
int32_t open(const uint8_t *filename)
{
    int open_fd;
    dentry_t file_dentry;

    // check if a file descriptor is available
    for (open_fd = 0; open_fd < FD_ARRAY_LENGTH; open_fd++)
    {
        if (current_pcb->fd_array[open_fd].flags != 1)
        {
            break;
        }
    }

    // if there is no fd available, return -1
    if (open_fd == FD_ARRAY_LENGTH)
    {
        return -1;
    }

    // read the directory to find the file
    if (read_dentry_by_name(filename, &file_dentry) != 0)
    {
        return -1;
    }

    if (file_dentry.fileType == RTC_FILE_TYPE)
    {
        // setup for rtc
        current_pcb->fd_array[open_fd].file_op_table_ptr = (int32_t *)&rtc_op_table;
        current_pcb->fd_array[open_fd].file_position = 0;
        current_pcb->fd_array[open_fd].inode_num = file_dentry.inodeNumber;
        current_pcb->fd_array[open_fd].flags = 1;
    }
    else if (file_dentry.fileType == DIRECTORY_FILE_TYPE)
    {
        // setup for directory
        current_pcb->fd_array[open_fd].file_op_table_ptr = (int32_t *)&dir_op_table;
        current_pcb->fd_array[open_fd].file_position = 0;
        current_pcb->fd_array[open_fd].inode_num = file_dentry.inodeNumber;
        current_pcb->fd_array[open_fd].flags = 1;
    }
    else
    {
        // setup for regular file
        current_pcb->fd_array[open_fd].file_op_table_ptr = (int32_t *)&file_op_table;
        current_pcb->fd_array[open_fd].file_position = 0;
        current_pcb->fd_array[open_fd].inode_num = file_dentry.inodeNumber;
        current_pcb->fd_array[open_fd].flags = 1;
    }

    // call open function
    int32_t (*open)(const uint8_t *) = (void *)current_pcb->fd_array[open_fd].file_op_table_ptr[0];
    (*open)(filename);

    // return file descriptor
    return open_fd;
}

/* int32_t close(int32_t fd)
 * Inputs:      fd - file descriptor of file to close
 * Return Value: 0 on success, -1 on failure
 * Function: closes file and empties FD array element */
int32_t close(int32_t fd)
{
    // valid file descriptor
    if (fd <= STDOUT_FD || fd >= FD_ARRAY_LENGTH)
    {
        return -1;
    }
    else if (current_pcb->fd_array[fd].flags != 1)
    {
        return -1;
    }
    else
    {
        // call appropriate close function
        int32_t (*close)(int32_t) = (void *)current_pcb->fd_array[fd].file_op_table_ptr[3];
        (*close)(fd);

        // clear fd array element
        current_pcb->fd_array[fd].file_op_table_ptr = 0;
        current_pcb->fd_array[fd].file_position = 0;
        current_pcb->fd_array[fd].flags = 0;
        current_pcb->fd_array[fd].inode_num = 0;

        return 0;
    }
}

/* int32_t gerargs(uint8_t *buf, int32_t nbytes)
 * Inputs:      buf - string to write, nbytes - number of bytes to write
 * Return Value: 0 on success, -1 on failure
 * Function: gets the args of the current pcb and puts them into the buffer */
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    //parameter checks
    if (buf == NULL || nbytes <= 0) {
        return -1;
    }

    //copies current args into the buffer, restricted by the nbytes
    strncpy((int8_t*)buf, (int8_t*)current_pcb->arg, nbytes);

    return 0;
}

/* int32_t vidmap(uint8_t** screen_start)
 * Inputs:      screen_start -- memory location to write video memory into
 * Return Value: 0 on success, -1 on failure
 * Function: maps text-mode video memory into user space */
int32_t vidmap(uint8_t** screen_start)
{
    // check for invalid inputs
    if ((uint32_t)screen_start == NULL) {
        return -1;
    } else if ((uint32_t)screen_start < ADDR_128MB || (uint32_t)screen_start > (ADDR_128MB + FOUR_MB)) {
        return -1;
    }

    *screen_start = (uint8_t*)(VIDMAP_VMEM_LOC * FOUR_MB + (VIDMAP_VMEM_LOC_PAGE * FOUR_KB));

    return 0;
}

int32_t set_handler(int32_t signum, void *handler_address)
{
    // Not implemented
    return -1;
}

int32_t sigreturn(void)
{
    // Not implemented
    return -1;
}

/* void init_current_pcb()
 * Inputs:      None
 * Return Value: Void
 * Function: sets up PCB for testing purposes */
void init_current_pcb()
{
    int i;
    for (i = 0; i < FD_ARRAY_LENGTH; i++)
    {
        fd_element_t empty = {.file_op_table_ptr = 0, .file_position = 0, .flags = 0, .inode_num = 0};
        current_pcb->fd_array[i] = empty;
    }
    current_pcb->process_id = 0;
    current_pcb->parent_process_id = 0;

    // mark first 2 FDs as in use
    current_pcb->fd_array[0].flags = 1;
    current_pcb->fd_array[1].flags = 1;
}

/* void flush_tlb()
 * Inputs:      None
 * Return Value: Void
 * Function: Flushes TLB */
void flush_tlb()
{
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        :
        : "%eax");
}

/* pcb_t* get_pcb_ptr(int32_t pid)
 * Inputs:      pid -- process ID of the requested PCB
 * Return Value: pointer to the PCB
 * Function: calculates offset and gets pointer to PCB */
pcb_t* get_pcb_ptr(int32_t pid) {
    return (pcb_t*)(EIGHT_MB - (EIGHT_KB * (pid + 1)));
}

/* int32_t get_pid()
 * Inputs:      None
 * Return Value: next available process_id, if none available then -1
 * Function: checks PCBs to see what process IDs are in use */
int32_t get_pid() {
    int i;
    for (i = 0; i < NUM_PIDS; i++) {
        if (get_pcb_ptr(i)->in_use != 1) {
            return i;
        }
    }
    
    return -1;
}

/* uint32_t get_phys_addr(int32_t pid)
 * Inputs:      pid -- process ID
 * Return Value: 20 MSB of the physical address of the program
 * Function: calculates offset and gets address to physical program memory */
uint32_t get_phys_addr(int32_t pid) {
    return (uint32_t)((EIGHT_MB + (FOUR_MB * pid)) >> PAGING_OFFSET);
}
