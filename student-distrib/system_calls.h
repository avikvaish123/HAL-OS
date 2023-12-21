#include "types.h"

#define FD_ARRAY_LENGTH             8
#define RTC_FILE_TYPE               0
#define DIRECTORY_FILE_TYPE         1
#define ARGS_SIZE                   32
#define EXEC_ARG_LEN                128
#define USER_SPACE_DIR_NUM 32


typedef struct term_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} term_table_t;

typedef struct file_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(uint32_t inode_idx, int32_t file_index, void* buf, int32_t num_bytes);
    int32_t (*write)(int32_t file_index, const void* buf, int32_t num_bytes);
    int32_t (*close)(int32_t fd);
} file_table_t;

typedef struct rtc_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} rtc_table_t;

typedef struct dir_table_t {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t file_index, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} dir_table_t;

typedef struct fd_element_t
{
    int32_t * file_op_table_ptr;
    uint32_t inode_num;
    uint32_t file_position;
    uint32_t flags;
} fd_element_t;

typedef struct pcb_t {
    fd_element_t fd_array[FD_ARRAY_LENGTH];
    int32_t process_id;
    int32_t parent_process_id;
    uint32_t esp_val;
    uint32_t ebp_val;
    uint32_t in_use;
    uint8_t arg[EXEC_ARG_LEN];
    uint32_t scheduling_esp_val;
    uint32_t scheduling_ebp_val;
} pcb_t;

volatile pcb_t* current_pcb;

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
void init_current_pcb();
void flush_tlb();
pcb_t* get_pcb_ptr(int32_t pid);
int32_t get_pid();
uint32_t get_phys_addr(int32_t pid);
