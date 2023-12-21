#include "scheduler.h"
#include "lib.h"
#include "paging.h"
#include "system_calls.h"
#include "i8259.h"
#include "devices/keyboard.h"
#include "devices/terminal.h"

uint8_t first_swap = 1;

/* void terminal_switch(uint8_t target_terminal)
 * 
 * This function is used for switching between active terminals displayed to the user
 * 
 * Inputs: target_terminal -- terminal to switch to
 * Return Value: None
 * Function: switches terminals
 */
void
terminal_switch(uint8_t target_terminal) {

    if (active_terminal == target_terminal) {
        return;
    }

    // update video paging to point to video if we're not pointing there
    if (active_terminal != scheduled_terminal) {
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        flush_tlb();
    }

    // copy video mem into active terminal video mem
    memcpy((void*)(TERMINAL_VIDEO + active_terminal*ALIGN_4KB), (void*)VIDEO, ALIGN_4KB);

    // copy target page into video mem
    memcpy((void*)VIDEO, (void*)(TERMINAL_VIDEO + target_terminal*ALIGN_4KB), ALIGN_4KB);

    // update paging to point to scheduled terminal after shifting video
    if (target_terminal != scheduled_terminal) {
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        flush_tlb();
    }

    // update active terminal
    active_terminal = target_terminal;    

    // execute shell on swap if first time
    if (!initialized_terminals[active_terminal]){
        send_eoi(KEYBOARD_IRQ);
        sti();
        execute((uint8_t*)"shell");
    }

}

/* void scheduler()
 * 
 * This function is used for switching between scheduled terminals
 * 
 * Inputs: None
 * Return Value: None
 * Function: switches which program is running
 */

void
scheduler() {
    /*
    1. save ebp, esp
    2. setup iret context for terminal you're currently in
    3. swap ebp, esp to new terminal (swap tss.esp0 as well)
    4. switch program image
    5. switch video memory
    */
    uint8_t next_scheduled_terminal;
    uint32_t ebp_temp_val, esp_temp_val;

    if (first_swap) { 
        scheduled_terminal = 0;
        first_swap = 0;
    }

    // save ebp, esp
    register uint32_t stored_esp asm("esp");
    register uint32_t stored_ebp asm("ebp");

    current_pcb->scheduling_esp_val = stored_esp;
    current_pcb->scheduling_ebp_val = stored_ebp;

    // update terminal positions
    terminals[scheduled_terminal].terminal_screen_x = get_screen_x();
    terminals[scheduled_terminal].terminal_screen_y = get_screen_y();

    // find next scheduled terminal/pcb to run
    next_scheduled_terminal = (scheduled_terminal + 1) % MAX_TERMINALS;

    while (!initialized_terminals[next_scheduled_terminal]) {
        next_scheduled_terminal++;
        next_scheduled_terminal %= MAX_TERMINALS;
    }

    scheduled_terminal = next_scheduled_terminal;

    // context switch
    current_pcb = get_pcb_ptr(terminals[scheduled_terminal].active_pid);
    tss.esp0 = EIGHT_MB - (EIGHT_KB * current_pcb->process_id) - sizeof(int);

    // update program page
    directoryArray[USER_SPACE_DIR_NUM].offset_31_12 = get_phys_addr(current_pcb->process_id);
    flush_tlb();

    // remap video pages
    if (scheduled_terminal == active_terminal) {
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;
        flush_tlb();
    } else {
        tableArray[VIDEO_MEMORY_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = (TERMINAL_VIDEO + (ALIGN_4KB*scheduled_terminal)) >> PAGING_OFFSET;
        flush_tlb();
    }

    // update the screen_x and screen_y when switching terminals
    set_cursor(terminals[scheduled_terminal].terminal_screen_x, terminals[scheduled_terminal].terminal_screen_y);

    ebp_temp_val = current_pcb->scheduling_ebp_val;
    esp_temp_val = current_pcb->scheduling_esp_val;

    asm volatile(
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        "leave;"
        "ret;"
        :
        : "r"(ebp_temp_val), "r"(esp_temp_val)
        : "%ebp", "%esp");
}
