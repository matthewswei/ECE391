# paging_registers.S - Set up register values for paging
# vim:ts=4 noexpandtab

.text
.globl set_control_registers

# Magic Numbers
# 8 -- Value to get argument from stack
# 0x10 -- Mask bor setting mixed paging sizes
# $0x80000000 -- Mask for paging enable paging bit

# set_control_registers
#   DESCRIPTION:    Correctly set the control registers to allow for paging
#   INPUTS:         page_dir  -- pointer to the head of the page directory 
#   OUTPUTS:       None
#   RETURN VALUE:  None
#   SIDE EFFECTS:   Stores the page directory address in CR3 and sets the
#                   other control registers needed for paging 
set_control_registers:
    PUSHL %EBP                 # set up stack frame
    MOVL %ESP, %EBP

    MOVL 8(%ESP), %EAX        # store the address of the page directory into EAX
    MOVL %EAX, %CR3           # store the page directory's adress in Cr3 

    MOVL %CR4, %EAX         
    ORL $0x10, %EAX         # set bit 4 of CR4 to high (0 indexed)
    MOVL %EAX, %CR4         # allow mixed page sizes (pages of size 4kB and 4MB)

    MOVL %CR0, %EAX
    ORL $0x80000000, %EAX   # set paging optional bit in Cr0
    MOVL %EAX, %CR0         # enable paging

    leave                   # break down stack frame
    ret
