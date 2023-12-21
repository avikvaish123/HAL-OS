#include "lib.h"
#include "x86_desc.h"
#include "interrupts.h"
#include "intr_asm_linkage.h"
#include "sys_call_asm_linkage.h"
#include "i8259.h"
#include "devices/pit.h"


//lookup table for exception messages
const char* exception_lookup[NUM_EXCEPTIONS]= {
    "EXCEPTION 0: Divide Error Exception",
    "EXCEPTION 1: Debug Exception",
    "EXCEPTION 2: NMI Interrupt",
    "EXCEPTION 3: Breakpoint Exception",
    "EXCEPTION 4: Overflow Exception",
    "EXCEPTION 5: BOUND Range Exceeded Exception",
    "EXCEPTION 6: Invalid Opcode Exception",
    "EXCEPTION 7: Device Not Available Exception",
    "EXCEPTION 8: Double Fault Exception",
    "EXCEPTION 9: Coprocessor Segment Overrun",
    "EXCEPTION 10: Invalid TSS Exception",
    "EXCEPTION 11: Segment Not Present",
    "EXCEPTION 12: Stack Fault Exception",
    "EXCEPTION 13: General Protection Exception",
    "EXCEPTION 14: Page Fault Exception",
    "EXCEPTION 15: Assertion Error",
    "EXCEPTION 16: FPU Floating-Point Error",
    "EXCEPTION 17: Alignment Check Exception",
    "EXCEPTION 18: Machine-Check Exception",
    "EXCEPTION 19: SIMD Floating-Point Exception"
};

//handler array for holding all the handlers during init
void (*handlers[NUM_VEC])();

//handler array for holding all the exception handlers
void (*exceptions[NUM_EXCEPTIONS])() = {
    divide_error_exc,
    debug_exc,
    nmi_interrupt_exc,
    breakpoint_exc,
    overflow_exc,
    bound_range_exceeded_exc,
    invalid_opcode_exc,
    device_not_available_exc,
    double_fault_exc,
    coprocessor_segment_overrun_exc,
    invalid_tss_exc,
    segment_not_present_exc,
    stack_fault_exc,
    general_protection_exc,
    page_fault_exc,
    assertion_error_exc,
    fpu_fp_error_exc,
    alignment_check_exc,
    machine_check_exc,
    simd_fp_exc
};

/* void init_interrupts(void)
 * Inputs:      void
 * Return Value: void
 * Function: initializes IDT */
void 
init_interrupts() {
    int i;
    // allocate handlers for exceptions in memory
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
        handlers[i] = exceptions[i];
    }

    // allocate rest of handlers to NULL for time being
    for (i = NUM_EXCEPTIONS; i < NUM_VEC; i++) {
        // assign specific handlers for specifc addresses in the IDT
        if (i == SYSTEM_CALL_INDEX) {
            handlers[i] = sys_call_linkage;
            continue;
        }

        if (i == RTC_INDEX) {
            handlers[i] = rtc_intr;
            continue;
        }

        if (i == KEYBOARD_INDEX) {
            handlers[i] = keyboard_intr;
            continue;
        }

        if (i == PIT_INDEX) {
            handlers[i] = pit_intr;
            continue;
        }

        handlers[i] = NULL;
    }

    // initialize all entries in idt
    // all the handlers will be in KERNEL_CS
    // offset is determined by address of handler function
    for (i = 0; i < NUM_VEC; i++) {
        SET_IDT_ENTRY(idt[i], handlers[i]);
        idt[i].seg_selector = KERNEL_CS;
        if (handlers[i]) {
            idt[i].present = 1;
        } else {
            idt[i].present = 0;
        }

        // change priority level for system call
        if (i == SYSTEM_CALL_INDEX) {
            idt[i].dpl = USER_PRIVILEGE_LEVEL;
        } else {
            idt[i].dpl = KERNEL_PRIVILEGE_LEVEL;
        }

        idt[i].reserved0 = 0;

        // set gate type as trap for system call
        if (i == SYSTEM_CALL_INDEX) {
            idt[i].size = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 1;
        } else if (i < NUM_EXCEPTIONS) {
            // set gate type as trap for exceptions
            idt[i].size = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 1;
        } else {
            // set gate type as trap if not system call
            idt[i].size = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 1;
        }
     }
}


/* void exception_handler(int exc_num)
 * Inputs:      int exec_num -> which exception to print
 * Return Value: void
 * Function: prints error message to console */
void
exception_handler(int exc_num) {
    printf(" %s", exception_lookup[exc_num]);
    exception_in_child = 1;
    
    asm volatile ("jmp halt");
}
