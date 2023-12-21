#define ASM     1

# numbers for each exception
DIVIDE_ERROR_EXCEPTION_NUM = 0x00
DEBUG_EXCEPTION_NUM = 0x01
NMI_INTERRUPT_NUM = 0x02
BREAKPOINT_EXCEPTION_NUM = 0x03
OVERFLOW_EXCEPTION_NUM = 0x04
BOUND_RANGE_EXCEPTION_NUM = 0x05
INVALID_OPCODE_EXCEPTION_NUM = 0x06
DEVICE_NOT_AVAIL_EXCEPTION_NUM = 0x07
DOUBLE_FAULT_EXCEPTION_NUM = 0x08
COPROCESSOR_SEG_OVERRUN_NUM = 0x09
INVALID_TSS_EXCEPTION_NUM = 0x0A
SEGMENT_NOT_PRESENT_NUM = 0x0B
STACK_FAULT_EXCEPTION_NUM = 0x0C
GENERAL_PROTECTION_EXCEPTION_NUM = 0x0D
PAGE_FAULT_EXCEPTION_NUM = 0x0E
ASSERTION_EXCEPTION_NUM = 0x0F
FPU_FP_EXCEPTION_NUM = 0x10
ALIGNMENT_CHECK_EXCEPTION_NUM = 0x11
MACHINE_CHECK_EXCEPTION = 0x12
SIMD_FP_EXCEPTION_NUM = 0x13

# export each handler wrapper to intr_asm_linkage.h
.globl divide_error_exc, debug_exc, nmi_interrupt_exc, breakpoint_exc, overflow_exc, bound_range_exceeded_exc, invalid_opcode_exc, device_not_available_exc, \
        double_fault_exc, coprocessor_segment_overrun_exc, invalid_tss_exc, segment_not_present_exc, stack_fault_exc, general_protection_exc, page_fault_exc, \
        assertion_error_exc, fpu_fp_error_exc, alignment_check_exc, machine_check_exc, simd_fp_exc, keyboard_intr, rtc_intr, system_call_intr, pit_intr

/* void divide_error_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for divide error */
divide_error_exc:
    pushfl                              # push EFLAGS
    pushal                              # push all registers
    pushl $DIVIDE_ERROR_EXCEPTION_NUM   # push exception number as argument
    call exception_handler              # call exception handler
    addl $4, %esp                       # remove argument from stack
    popal                               # restore all registers
    popfl                               # restore EFLAGS
    iret                                # return

/* void debug_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for debug exception */
debug_exc:
    pushfl
    pushal
    pushl $DEBUG_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void nmi_interrupt_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for non-maskable interrupt exception */
nmi_interrupt_exc:
    pushfl
    pushal
    pushl $NMI_INTERRUPT_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void breakpoint_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for breakpoint exception */
breakpoint_exc:
    pushfl
    pushal
    pushl $BREAKPOINT_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void overflow_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for overflow exception */
overflow_exc:
    pushfl
    pushal
    pushl $OVERFLOW_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void bound_range_exceeded_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for bound range exceeded exception */
bound_range_exceeded_exc:
    pushfl
    pushal
    pushl $BOUND_RANGE_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void invalid_opcode_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for invalid opcode exception */
invalid_opcode_exc:
    pushfl
    pushal
    pushl $INVALID_OPCODE_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void device_not_available_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for device not available exception */
device_not_available_exc:
    pushfl
    pushal
    pushl $DEVICE_NOT_AVAIL_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void double_fault_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for double fault exception */
double_fault_exc:
    pushfl
    pushal
    pushl $DOUBLE_FAULT_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void coprocessor_segment_overrun_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for coprocessor segment overrun exception */
coprocessor_segment_overrun_exc:
    pushfl
    pushal
    pushl $COPROCESSOR_SEG_OVERRUN_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void invalid_tss_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for invalid task state segment exception */
invalid_tss_exc:
    pushfl
    pushal
    pushl $INVALID_TSS_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void segment_not_present_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for segment not present exception */
segment_not_present_exc:
    pushfl
    pushal
    pushl $SEGMENT_NOT_PRESENT_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void stack_fault_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for stack fault exception */
stack_fault_exc:
    pushfl
    pushal
    pushl $STACK_FAULT_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void general_protection_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for general protection exception */
general_protection_exc:
    pushfl
    pushal
    pushl $GENERAL_PROTECTION_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void page_fault_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for page fault exception */
page_fault_exc:
    pushfl
    pushal
    pushl $PAGE_FAULT_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void assertion_error_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for assertion error exception */
assertion_error_exc:
    pushfl
    pushal
    pushl $ASSERTION_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void fpu_fp_error_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for FPU floating-point error exception */
fpu_fp_error_exc:
    pushfl
    pushal
    pushl $FPU_FP_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void alignment_check_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for alignment check exception */
alignment_check_exc:
    pushfl
    pushal
    pushl $ALIGNMENT_CHECK_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void machine_check_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for machine check exception */
machine_check_exc:
    pushfl
    pushal
    pushl $MACHINE_CHECK_EXCEPTION
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void simd_fp_exc()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for SIMD floating-point exception */
simd_fp_exc:
    pushfl
    pushal
    pushl $SIMD_FP_EXCEPTION_NUM
    call exception_handler
    addl $4, %esp
    popal
    popfl
    iret

/* void keyboard_intr()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for keyboard interrupt */
keyboard_intr:
    pushfl
    pushal
    call keyboard_handler
    popal
    popfl
    iret

/* void rtc_intr()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for RTC (real-time clock) interrupt */
rtc_intr:
    pushfl
    pushal
    call rtc_handler
    popal
    popfl
    iret

/* void pit_intr()
 * Inputs: None
 * Return Value: None
 * Function: wrapper for PIT interrupt */
pit_intr:
    pushfl
    pushal
    call pit_handler
    popal
    popfl
    iret
