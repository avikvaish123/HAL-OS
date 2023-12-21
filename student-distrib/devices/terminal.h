#include "../lib.h"

#define MAX_BUFFER_LENGTH       128
#define NUM_COLS                80
#define NUM_ROWS                25
#define SPACE_ASCII             0x20
#define NULL_ASCII              0x00
#define NEWLINE_ASCII           0x0A
#define BACKSPACE_ASCII         0x08
#define KEYBOARD_ROW_START      0
#define KEYBOARD_ROW_END        1
#define TERMINAL_ROW_START      2
#define TERMINAL_ROW_END        3
#define MAX_INPUT_BUF_LEN       1024
#define MAX_TERMINALS           3

typedef struct terminal_info_t {
    char keyboard_buffer[MAX_BUFFER_LENGTH+1];
    volatile uint32_t current_char;
    volatile uint8_t enter_flag;
    volatile uint8_t current_line;
    volatile uint8_t reset_flag;
    volatile uint8_t char_in_line;
    volatile uint32_t last_current_char;
    volatile int terminal_screen_x;
    volatile int terminal_screen_y;
    volatile uint8_t active_pid;
}terminal_info_t;

terminal_info_t terminals[MAX_TERMINALS];

/* open terminal */
int32_t terminal_open(const uint8_t* filename);

/* close terminal */
int32_t terminal_close(int32_t fd);

/* read keyboard input */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

/* write buf to terminal */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

/* update the keyboard buffer with input */
void update_kb_buffer(char input);

/* clears the screen after CTRL-L */
void clear_terminal();
