#include "types.h"

#define MAX_TERMINALS               3
#define TERMINAL_1                  0
#define TERMINAL_2                  1
#define TERMINAL_3                  2
#define EIGHT_MB                    0x800000
#define EIGHT_KB                    0x2000
#define USER_ESP                    0x8400000 - 4

// active terminal can either be 0, 1, or 2
volatile uint8_t active_terminal;

// sheduled terminal can either be 0, 1, or 2
volatile uint8_t scheduled_terminal;

// status of terminals
uint8_t initialized_terminals[MAX_TERMINALS];

void terminal_switch(uint8_t target_terminal);
void scheduler();
