#include "../types.h"

#define PIT_LINE            0
#define PIT_CHANNEL_0       0x40
#define PIT_CHANNEL_1       0x41
#define PIT_CHANNEL_2       0x42
#define PIT_COMMAND         0x43
#define PIT_MODE            0x36
#define PIT_INDEX           0x20
#define EIGHT               8
#define PIT_MASK            0xFF

#define PIT_FREQ_CONSTANT   1193181
#define PIT_100HZ           PIT_FREQ_CONSTANT / 100
#define PIT_20HZ            PIT_FREQ_CONSTANT / 20

void init_pit();
void pit_handler();
