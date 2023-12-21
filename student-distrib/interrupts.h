#include "lib.h"
#include "x86_desc.h"

#define NUM_EXCEPTIONS      0x20
#define SYSTEM_CALL_INDEX   0x80

#define USER_PRIVILEGE_LEVEL    3
#define KERNEL_PRIVILEGE_LEVEL  0

uint8_t exception_in_child;

void init_interrupts();
void exception_handler(int exc_num);
