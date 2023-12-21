/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    uint32_t saved_flags;
    cli_and_save(saved_flags);

    /* program PICs */
    outb(ICW1, PIC1_COMMAND);
    outb(ICW1, PIC2_COMMAND);
    outb(ICW2_MASTER, PIC1_DATA);
    outb(ICW2_SLAVE, PIC2_DATA);
    outb(ICW3_MASTER, PIC1_DATA);
    outb(ICW3_SLAVE, PIC2_DATA);
    outb(ICW4, PIC1_DATA);
    outb(ICW4, PIC2_DATA);

    /* restore masks */
    outb(master_mask, PIC1_DATA);
    outb(slave_mask, PIC2_DATA);

    /* enable IRQ2 for secondary PIC */
    enable_irq(SLAVE_PORT);

    restore_flags(saved_flags);

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;

    if(irq_num < PORT_COUNT) {
        port = PIC1_DATA;
        master_mask &= ~(1 << irq_num);
        value = master_mask;
    } else {
        port = PIC2_DATA;
        slave_mask &= ~(1 << (irq_num - PORT_COUNT));
        value = slave_mask;
    }
    outb(value, port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    uint8_t value;

    if(irq_num < PORT_COUNT) {
        port = PIC1_DATA;
        master_mask |= (1 << irq_num);
        value = master_mask;
    } else {
        port = PIC2_DATA;
        slave_mask |= (1 << (irq_num - PORT_COUNT));
        value = slave_mask;
    }
    outb(value, port);
    
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {

    uint8_t data = EOI;

    /* Parameter check*/
    if(irq_num >= TOTAL_PORT_COUNT) {
        return;
    }

    /*if condition sends to both slave and master pic, else sends only to slave*/
    if(irq_num >= PORT_COUNT) {
        outb(data|(irq_num - PORT_COUNT), PIC2_COMMAND);
        outb(data|SLAVE_PORT, PIC1_COMMAND);
    } else {
        outb(data|irq_num, PIC1_COMMAND);
    }

}
