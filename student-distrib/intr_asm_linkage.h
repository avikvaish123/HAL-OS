#include "interrupts.h"
#include "devices/keyboard.h"
#include "devices/rtc.h"

// export each handler wrapper to interrupts.c
extern void divide_error_exc();
extern void debug_exc();
extern void nmi_interrupt_exc();
extern void breakpoint_exc();
extern void overflow_exc();
extern void bound_range_exceeded_exc();
extern void invalid_opcode_exc();
extern void device_not_available_exc();
extern void double_fault_exc();
extern void coprocessor_segment_overrun_exc();
extern void invalid_tss_exc();
extern void segment_not_present_exc();
extern void stack_fault_exc();
extern void general_protection_exc();
extern void page_fault_exc();
extern void assertion_error_exc();
extern void fpu_fp_error_exc();
extern void alignment_check_exc();
extern void machine_check_exc();
extern void simd_fp_exc();
extern void keyboard_intr();
extern void rtc_intr();
extern void pit_intr();
