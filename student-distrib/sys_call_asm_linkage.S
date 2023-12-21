#define ASM     1

.globl sys_call_linkage, flush_tlb


// system_call_linkage()
// Inputs: EAX
// Outputs: None
// Side Effects: Executes system call based on EAX - 1 
sys_call_linkage:
        # save registers
        pushl %ebp
        pushl %edi
        pushl %esi
        
        pushfl

        pushl %edx
        pushl %ecx
        pushl %ebx
        
        # check for valid system call number
        cmpl $1, %eax
        jl invalid_sys_call
        cmpl $10, %eax
        jg invalid_sys_call

        # reduce system call number by 1 for jump table
        decl %eax

        # call appropriate system call
        call *sys_call_table(, %eax, 4)

        jmp done

    invalid_sys_call:
        # restore registers
        movl $-1, %eax
    done:
        addl $12, %esp

        popfl
        
        popl %esi
        popl %edi
        popl %ebp

        iret

sys_call_table: 
		.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
