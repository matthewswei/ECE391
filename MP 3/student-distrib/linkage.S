# linkage.S - Sets up assembly linkage for handlers
# vim:ts=4 noexpandtab

#define ASM 1

#include "linkage.h"
.globl kb_linkage, rtc_linkage, pit_linkage

# #define INTR_LINK(name, func)       \
#     .global name                   ;\
#     name:                          ;\
#         pushal                     ;\
#         pushfl                     ;\
#         call func                  ;\
#         popfl                      ;\
#         popal                      ;\
#         iret

# @ INTRA_LINK(DE_handler_linkage, handle_DE);
# @ INTRA_LINK(DB_handler_linkage, handle_DB);
# @ INTRA_LINK(NMI_handler_linkage, handle_NMI);
# @ INTRA_LINK(BP_handler_linkage, handle_BP);
# @ INTRA_LINK(OF_handler_linkage, handle_OF);
# @ INTRA_LINK(BR_handler_linkage, handle_BR);
# @ INTRA_LINK(UD_handler_linkage, handle_UD);
# @ INTRA_LINK(NM_handler_linkage, handle_NM);
# @ INTRA_LINK(DF_handler_linkage, handle_DF);
# @ INTRA_LINK(CSO_handler_linkage, handle_CSO);
# @ INTRA_LINK(TS_handler_linkage, handle_TS);
# @ INTRA_LINK(NP_handler_linkage, handle_NP);
# @ INTRA_LINK(SS_handler_linkage, handle_SS);
# @ INTRA_LINK(GP_handler_linkage, handle_GP);
# @ INTRA_LINK(PF_handler_linkage, handle_PF);
# @ INTRA_LINK(MF_handler_linkage, handle_MF);
# @ INTRA_LINK(AC_handler_linkage, handle_AC);
# @ INTRA_LINK(MC_handler_linkage, handle_MC);
# @ INTRA_LINK(XF_handler_linkage, handle_XF);

# INTRA_LINK(sys_handler_linkage, sys_handler);

# Pushes all registers and flags, calls the keyboard handler function,
#   restores the values of the registers and flags, and returns from interrupt
kb_linkage:
    pushal
    pushfl
    call keyboard_handler
    popfl
    popal
    iret

# Pushes all registers and flags, calls the RTC handler function, restores
#   the values of the registers and flags, and returns from interrupt
rtc_linkage:
    pushal
    pushfl
    call rtc_handler
    popfl
    popal
    iret

pit_linkage:
    pushal
    pushfl
    call pit_handler
    popfl
    popal
    iret

    