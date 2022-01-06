/* idt.h - Defines for IDT initializaton
vim:ts=4 noexpandtab
*/

#ifndef _IDT_H
#define _IDt_H

#define keyboard_irq 33
#define rtc_irq      40
#define pit_idt 0x20


void DE_exception();
void DB_exception();
void NMI_exception();
void BP_exception();
void OF_exception();
void BR_exception();
void UD_exception();
void NM_exception();
void DF_exception();
void CSO_exception();
void TS_exception();
void NP_exception();
void SS_exception();
void GP_exception();
void PF_exception();
void MF_exception();
void AC_exception();
void MC_exception();
void XM_exception();
void VE_exception();
void SX_exception();
void General_exception();
extern void handle_system_call(void);
int exception_flag;

/* Make IDT initialized */
void idt_init();
#endif
