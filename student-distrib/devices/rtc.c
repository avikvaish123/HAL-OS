#include "rtc.h"
#include "../lib.h"
#include "../i8259.h"
#include "../tests.h"


volatile uint32_t rtc_interrupt_flag = 0;

void set_rtc_frequency(uint32_t frequency);

/* void rtc_init()
 * Inputs:      void
 * Return Value: void
 * Function: enables interrupts for the RTC
 */
void rtc_init()
{
    uint8_t prev, rate;
    cli();

    // turn on periodic interrupts
    outb(RTC_REG_B_NMI_BIT, RTC_INDEX_PORT);
    prev = inb(RTC_DATA_PORT);
    outb(RTC_REG_B_NMI_BIT, RTC_INDEX_PORT);
    outb(prev | RTC_BIT_6_ENABLE, RTC_DATA_PORT);

    // change interrupt rate, mask ensures its always <= 15
    rate = RTC_RATE & RTC_RATE_MASK;

    //setting rate to minimum
    outb(RTC_REG_A_NMI_BIT, RTC_INDEX_PORT);
    prev = inb(RTC_DATA_PORT);
    outb(RTC_REG_A_NMI_BIT, RTC_INDEX_PORT);
    outb((prev & RTC_RATE_BITS) | rate, RTC_DATA_PORT);

    // read from register C to allow for more interrupts
    outb(RTC_REG_C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    enable_irq(RTC_IRQ);
    sti();
}

/* void rtc_handler(void)
 * Inputs:      void
 * Return Value: void
 * Function: calls test_interrupt function */
void rtc_handler()
{
    cli();

#if (RTC_TEST)
    // test_interrupts();
    puts("1");
#endif

    // read from register C to allow for more interrupts
    outb(RTC_REG_C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    // when rtc handler is called, sets the rtc intterupt flag to high
    if (rtc_interrupt_flag == 0)
    {
        rtc_interrupt_flag = 1;
    }

    sti();

    send_eoi(RTC_IRQ);
}

/* int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes)
 * Inputs:  int32_t fd (file descriptor number), void* buf (output buffer),
            int32_t nbytes (number of bytes to be read)
 * Return Value: 0 on success or -1 on failure
 * Function: Wait until a new RTC int is recieved 
 * Documentation requirements: Make sure that rtc read must return only after an RTC interrupt has occurred. 
 * You might want to use some sort of flag here.
 * */
int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes)
{
    //sets interrupt flag to 0 and waits for RTC handler to be called
    rtc_interrupt_flag = 0;
    while (rtc_interrupt_flag == 0)
    {
    }
    return 0;
}

/* int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes)
 * Inputs:  int32_t fd (file descriptor number), void* buf (output buffer),
            int32_t nbytes (number of bytes to be read)
 * Return Value: number of bytes written
 * Function: Writes a new frequency stored in buf to the RTC */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes)
{

    uint32_t frequency;

    //parameter checks
    if (nbytes != NBYTES_SIZE || buf == NULL)
    {
        return -1;
    }
    else
    {
        //sets frequency from the buffer value
        frequency = *((uint32_t *)buf);
        set_rtc_frequency(frequency);
    }

    // read from register C to allow for more interrupts
    outb(RTC_REG_C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    send_eoi(RTC_IRQ);

    return nbytes;
}

/* int32_t rtc_open (const uint8_t* filename)
 * Inputs: int32_t fd (file descriptor number)
 * Return Value: 0 on success or -1 on failure
 * Function: closes the RTC device */
int32_t rtc_open(const uint8_t *filename)
{
    rtc_init();
    return 0;
}

/* int32_t rtc_close (int32_t fd)
 * Inputs: int32_t fd (file descriptor number)
 * Return Value: 0 on success or -1 on failure
 * Function: closes the RTC device */
int32_t rtc_close(int32_t fd)
{
    // disable_irq(RTC_IRQ);
    return 0;
}


/* void set_rtc_frequency(uint32_t frequency)
 * Inputs: uint32_t frequency (power of 2 from 2 to 1024)
 * Return Value: N/A
 * Function: converts a frequncy to the correct hertz rate and sends to register */
void set_rtc_frequency(uint32_t frequency)
{

    char prev;
    char rate;

    //case statement for getting the right rate
    switch (frequency)
    {
    case HERTZ_2:
        rate = RATE_2;
        break;
    case HERTZ_4:
        rate = RATE_4;
        break;
    case HERTZ_8:
        rate = RATE_8;
        break;
    case HERTZ_16:
        rate = RATE_16;
        break;
    case HERTZ_32:
        rate = RATE_32;
        break;
    case HERTZ_64:
        rate = RATE_64;
        break;
    case HERTZ_128:
        rate = RATE_128;
        break;
    case HERTZ_256:
        rate = RATE_256;
        break;
    case HERTZ_512:
        rate = RATE_512;
        break;
    case HERTZ_1024:
        rate = RATE_1024;
        break;
    default:
        return;
    }

    //sets rate based on previously claculated value
    outb(RTC_REG_A_NMI_BIT, RTC_INDEX_PORT);
    prev = inb(RTC_DATA_PORT);
    outb(RTC_REG_A_NMI_BIT, RTC_INDEX_PORT);
    outb((prev & RTC_RATE_BITS) | rate, RTC_DATA_PORT);
}
