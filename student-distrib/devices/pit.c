#include "pit.h"
#include "rtc.h"
#include "../lib.h"
#include "../i8259.h"
#include "../tests.h"
#include "../scheduler.h"
#include "../system_calls.h"
#include "terminal.h"



/* void pit_init()
 * Inputs:      void
 * Return Value: void
 * Function: enables interrupts for the PIT
 */
void init_pit() {
    outb(PIT_MODE, PIT_COMMAND);
    outb(PIT_20HZ & PIT_MASK , PIT_CHANNEL_0);
    outb((PIT_20HZ >> EIGHT) & PIT_MASK, PIT_CHANNEL_0);

    enable_irq(PIT_LINE);
    return;
}


/* void pit_handler(void)
 * Inputs:      void
 * Return Value: void
 * Function: calls scheduler function */
void pit_handler(void){
    send_eoi(PIT_LINE);
    if (current_pcb && initialized_terminals && terminals) {
        scheduler();
    }
}
