//NOTE! You might need to comment this all out!
#include "rtc.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"

volatile int read_flag;

//follos OSDEV
// https://urldefense.com/v3/__https://wiki.osdev.org/RTC__;!!DZ3fjg!tLjUW3Es0CZE6PP_sc5By2r1dJFPLxm86CGkfF1n3LM3OYV5RhKttcWbSR9haZ61$ 
/* Initialize the RTC */
//follows OSDEV logic
//credit is given to https://wiki.osdev.org/RTC#Programming_the_RTC
/*DESCRIPTION 
*   this function initializes the RTC
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= establishes the connection from the RTC with the PIC
*/
uint32_t rtc_init(){
    cli();
    char prev;
    
    outb(0x8A,RTC);             //0x8A is the port for A, and 8 is because it wants to mask it
    // outb(2,CMOS);         // not sure what should be written in port A right now (OSDEV says default is 6)
    outb(0x8B, RTC);            // 0x8B is the port for B, and 8 is because it wants to mask it
    prev = inb(CMOS);       
    outb(0x8B, RTC);            // 0x8B is the port for B, 8 because it wants to mask it
    outb(prev | 0x40 , CMOS);   // ORd with 0x40 because it wants to 4th bit to be on
    enable_irq(8);              // 8 is from OSDEV. which is the IRQ for RTC
    
    sti();
    return 0; //Return for success
}


void rtc_interrupt(){
    read_flag = 0;
    cli();
    //test_interrupts();
    outb(0x0C,RTC); // this is the offset for register C
    inb(CMOS);
    sti();
    send_eoi(8); //8 because it is the IRQ port for RTC
    return;

}


/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
 * int rtc_open()
 * Inputs: filename
 * Return Value: uint32_t
 * Function: Initializes the frequency rate to 2HZ*/
int32_t rtc_open(const uint8_t* filename){
    unsigned char rate = 0x0F;
    unsigned char prev;

    cli();
    /* start of critical section */

    outb(0x8A, RTC);  //0x8A is the port for A, and 8 is because it wants to mask it
    prev = inb(CMOS); // get init value of register A
    outb(0x8A, RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    outb((prev & 0xF0) | rate, CMOS); // output the 2Hz rate into the CMOS

    /* end of critical section */
    sti();
    return 0;
}

/* 
 * int rtc_close()
 * Inputs: fd
 * Return Value: int
 * Function: Does nothing for now 
 */
int32_t rtc_close(int32_t fd){
    return 0;
}

/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
 *              It changes the frequency of the interrupt based on the buffer pointer passed in and sets it according to algo provided above
 * int rtc_write()
 * Inputs: fd, buf, nbytes
 * Return Value: int
 * Function: Changes the rate based on the input and does 32768 >> (rate-1)*/
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes){
    // printf("ENTERS HERE");
    unsigned char temp_rate;
    unsigned char prev;
    temp_rate = *(unsigned char *)buf;
    // printf("rateeeeeeeeeeee curr: %s", temp_rate);
    temp_rate &= 0x0F;
    if(temp_rate <= 2 || temp_rate > 15){
        return -1;
        }
    
    cli();
    /* start of critical section */

    outb(0x8A, RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    prev = inb(CMOS);  // get init value of register A
    outb(0x8A, RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    outb((prev & 0xF0) | temp_rate, CMOS); // output the rate into the CMOS

    /* end of critical section */
    sti();

    return 0;

}

/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
 * int rtc_write()
 * Inputs: None
 * Return Value: int
 * Function: blocks until the next interrupt */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    read_flag = 1;
    while(read_flag){ }
    return 0;
}

/* uint32_t rtc_rate()
 * Inputs: frequency
 * Return Value: uint32_t
 * Function: sets the rtc rate based on the frequency*/
int32_t rtc_rate(int32_t freq){
    switch (freq){
        case 2:     return 0x0F;
        case 4:     return 0x0E;
        case 8:     return 0x0D;
        case 16:    return 0x0C;
        case 32:    return 0x0B;
        case 64:    return 0x0A;
        case 128:   return 0x09;
        case 256:   return 0x08;
        case 512:   return 0x07;
        case 1024:  return 0x06;
        default:    return -1;
    }
}

