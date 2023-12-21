#include "../types.h"

#define RTC_INDEX           0x28

#define RTC_INDEX_PORT                          0x70
#define RTC_DATA_PORT                           0x71
#define RTC_IRQ                                 8
#define RTC_REG_B_NMI_BIT                       0x8B
#define RTC_REG_A_NMI_BIT                       0x8A
#define RTC_BIT_6_ENABLE                        0x40
#define RTC_RATE_BITS                           0xF0
#define RTC_REG_A                               0x0A
#define RTC_REG_B                               0x0B
#define RTC_REG_C                               0x0C

#define RTC_RATE_MASK                           0x0F
#define RTC_PREV_MASK                           0xF0
#define RTC_RATE                                15
#define RTC_MIN_RATE                            3
#define RTC_MIN_FREQUENCY                       2
#define RTC_MAX_FREQUENCY                       1024

#define NBYTES_SIZE                             4

#define HERTZ_2                                 2
#define HERTZ_4                                 4
#define HERTZ_8                                 8
#define HERTZ_16                                16
#define HERTZ_32                                32
#define HERTZ_64                                64
#define HERTZ_128                               128
#define HERTZ_256                               256
#define HERTZ_512                               512
#define HERTZ_1024                              1024

#define RATE_2                                 0x0F
#define RATE_4                                 0x0E
#define RATE_8                                 0x0D
#define RATE_16                                0x0C
#define RATE_32                                0x0B
#define RATE_64                                0x0A
#define RATE_128                               0x09
#define RATE_256                               0x08
#define RATE_512                               0x07
#define RATE_1024                              0x06



void rtc_init();
void rtc_handler();

int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open (const uint8_t* filename);
int32_t rtc_close (int32_t fd);
