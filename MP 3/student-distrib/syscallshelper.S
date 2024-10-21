.text
.globl system_call, invalid, syscall_complete, jump_table, flush_tlb, iret_call, halt_return

system_call:
    # save other registers onto stack
    # base pointer
    pushl %ebp
    # stack pointer
    pushl %esp
    pushl %edi
    pushl %esi

    # put arguments onto stack
    pushl %edx
    pushl %ecx
    pushl %ebx

    # make sure current system call is valid
    cmpl $1, %eax
    jl invalid
    cmpl $8, %eax
    jg invalid

    # call system call
    call *jump_table(, %eax, 4)
    jmp syscall_complete

invalid:
    movl $-1, %eax

syscall_complete:
    # take arguments off stack
    popl %ebx
    popl %ecx
    popl %edx

    # remove other registers off of stack
    popl %edi
    popl %esi
    popl %esp
    popl %ebp

    iret

jump_table:
    # have a 0x0 at beginning since functions are 0 indexed
    .long 0x0, sys_halt, sys_exec, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap

flush_tlb:
    # flush tlb by moving regcr3 into regeax and then moving regeax back into regcr3
    mov %cr3, %eax
    mov %eax, %cr3
    ret

iret_call:
    pushl %ebp
    movl %esp, %ebp
    # set up the stack frame iret expects
    movl 8(%esp), %esi

    # data selector
	pushl $0x2B
    
    # current esp
	pushl $0x83FFFFC
    
    # eflags
	pushfl
    popl %edi
    orl $0x200, %edi
    pushl %edi

    # code selector (ring 3 code with bottom 2 bits set for ring 3)
	pushl $0x23
    
    # instruction address to return to
	pushl %esi 

	iret


halt_return:
    # set up stack frame
    pushl %ebp
    movl %esp, %ebp

    # get arguments off of stack; status into regeax, regebp into regebp, regesp into regesp
    movl 16(%esp), %eax
    movl 12(%esp), %ebp
    movl 8(%esp), %esp
    leave
    ret
