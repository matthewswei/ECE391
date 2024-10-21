/* linkage.h - Defines for various functions needed for assembly linkage
 * vim:ts=4 noexpandtab
 */

#ifndef _LINKAGE_H
#define _LINKAGE_H

#ifndef ASM

//extern void sys_handler_linkage();

/* Linkage for keyboard handler */
extern void kb_linkage();   

/* Linkage for RTC handler */
extern void rtc_linkage(); 

extern void pit_linkage();

#endif
#endif /* _LINKAGE_H */
