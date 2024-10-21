.text
.globl save_esp, save_ebp, restore_esp_ebp

# save_esp
#   DESCRIPTION:    Saves ESP value to EAX
#   INPUTS:         None
#   OUTPUTS:       None
#   RETURN VALUE:  None
#   SIDE EFFECTS:   Saves ESP value to EAX
#     
save_esp:
    movl %esp, %eax
    ret

# save_ebp
#   DESCRIPTION:    Saves EBP value to EAX
#   INPUTS:         None
#   OUTPUTS:       None
#   RETURN VALUE:  None
#   SIDE EFFECTS:   Saves EBP value to EAX
#     
save_ebp:
    movl %ebp, %eax
    ret

# restore_esp_ebp
#   DESCRIPTION:    Restores ESP and EBP values from argument to ESP and EBP registers
#   INPUTS:         ESP -- ESP value to restore to register
#                   EBP -- EBP value to restore to register
#   OUTPUTS:       None
#   RETURN VALUE:  None
#   SIDE EFFECTS:   Restores ESP and EBP values from argument to ESP and EBP registers
#     
restore_esp_ebp:
    pushl %ebp
    movl %esp, %ebp

    movl 12(%esp), %eax
    movl 8(%esp), %edx

    leave
    movl %eax, %ebp
    movl %edx, %esp
    ret
