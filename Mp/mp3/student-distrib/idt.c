/* idt.c - the C part of the IDT
 * vim:ts=4 noexpandtab
 */
 
#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "interrupt_handlers.h"

void idt_init(){
	/*Construct the IDT Entries*/

	int i;
	for(i  = 0; i <NUM_VEC; i++){
		idt[i].dpl = 0; // set priviledge to 0 for interrupts
		idt[i].seg_selector = KERNEL_CS; // setting segment delector field to kernel's code segment description from Appendix D
		idt[i].size = 1;
		idt[i].present = 1;
		idt[i].reserved0 = 0;
		idt[i].reserved1 = 1;
		idt[i].reserved2 = 1;
		idt[i].reserved3 = 1;
		idt[i].reserved4 = 0;
		
		if(i == 0x80){ // for systems call -- not defined yet
			idt[i].dpl = 3;
		}

		// if(i == 15 || ((i > 20) && (i < 30)) || i == 31 ){
		// 	idt[i].present = 0; // reserved vectors
		// }
		
		
		// if(i == 3 || i == 4){
		// 	idt[i].reserved0 = 1; // for trap gate
		// }
		if(i > 31 && i != 33 && i != 40){
			SET_IDT_ENTRY(idt[i], General_exception);
		}
		
		/*Interrupt from Keyboard*/
		if(i == keyboard_irq){
			SET_IDT_ENTRY(idt[i], handle_keyboard);
			idt[i].reserved3 = 0;
		}
		
		/*Interrupt from RTC*/
		if(i == rtc_irq){
			SET_IDT_ENTRY(idt[i], handle_rtc);
			idt[i].reserved3 = 0;
		}
	}

	SET_IDT_ENTRY(idt[0], DE_exception);
	SET_IDT_ENTRY(idt[1], DB_exception);
	SET_IDT_ENTRY(idt[2], NMI_exception);
	SET_IDT_ENTRY(idt[3], BP_exception);
	SET_IDT_ENTRY(idt[4], OF_exception);
	SET_IDT_ENTRY(idt[5], BR_exception);
	SET_IDT_ENTRY(idt[6], UD_exception);
	SET_IDT_ENTRY(idt[7], NM_exception);
	SET_IDT_ENTRY(idt[8], DF_exception);
	SET_IDT_ENTRY(idt[9], CSO_exception);
	SET_IDT_ENTRY(idt[10], TS_exception);
	SET_IDT_ENTRY(idt[11], NP_exception);
	SET_IDT_ENTRY(idt[12], SS_exception);
	SET_IDT_ENTRY(idt[13], GP_exception);
	SET_IDT_ENTRY(idt[14], PF_exception);
	SET_IDT_ENTRY(idt[16], MF_exception);
	SET_IDT_ENTRY(idt[17], AC_exception);
	SET_IDT_ENTRY(idt[18], MC_exception);
	SET_IDT_ENTRY(idt[19], XM_exception);
	SET_IDT_ENTRY(idt[20], VE_exception);
	SET_IDT_ENTRY(idt[30], SX_exception);
	
	/* to be linked once keyboard is completed*/
	// SET_IDT_ENTRY(idt[keyboard], keyboard handler);
	
}

void DE_exception() {
printf("Divide-by-zero exception occured");
while(1);
}
void DB_exception() {
printf("Debug exception occured");
while(1);
}
void NMI_exception() {
printf("NON-maskable Interrupt exception occured");
while(1);
}
void BP_exception() {
printf("Breakpoint exception occured");
while(1);
}
void OF_exception() {
printf("Overflow exception occured");
while(1);
}
void BR_exception() {
printf("Bound Range Exceeded exception occured");
while(1);
}
void UD_exception() {
printf("Invalid Opcode exception occured");
while(1);
}
void NM_exception() {
printf("Device Not Available exception occured");
while(1);
}
void DF_exception() {
printf("Double Fault exception occured");
while(1);
}
void CSO_exception() {
printf("Coprocessor Segment Overrun exception occured");
while(1);
}
void TS_exception() {
printf("Invalid TSS exception occured");
while(1);
}
void NP_exception() {
printf("Segment Not Present exception occured");
while(1);
}
void SS_exception() {
printf("Stack-Segment Fault exception occured");
while(1);
}
void GP_exception() {
printf("General Protection Fault exception occured");
while(1);
}
void PF_exception() {
printf("Page Fault exception occured");
while(1);
}
void MF_exception() {
printf("x87 Floating-Point Exception exception occured");
while(1);
}
void AC_exception() {
printf("Alignment Check exception occured");
while(1);
}
void MC_exception() {
printf("Machine Check exception occured");
while(1);
}
void XM_exception() {
printf("SIMD Floating-Point Exception occured");
while(1);
}
void VE_exception() {
printf("Virtualization Exception occured");
while(1);
}
void SX_exception() {
printf("Security Exception occured");
while(1);
}
void General_exception() {
printf("General Exception occured");
while(1);
}

