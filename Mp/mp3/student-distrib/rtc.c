//NOTE! You might need to comment this all out!
#include "rtc.h"
#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "terminal.h"

volatile int rtc_counters [NUM_TERMINALS];

//follos OSDEV
// https://urldefense.com/v3/__https://wiki.osdev.org/RTC__;!!DZ3fjg!tLjUW3Es0CZE6PP_sc5By2r1dJFPLxm86CGkfF1n3LM3OYV5RhKttcWbSR9haZ61$ 
/* Initialize the RTC */
//follows OSDEV logic
//credit is given to https://wiki.osdev.org/RTC#Programming_the_RTC
/*DESCRIPTION 
* this function initializes the RTC
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= establishes the connection from the RTC with the PIC
*/
uint32_t rtc_init(){
    rtc_counters[0] = 0;
    rtc_counters[1] = 0;
    rtc_counters[2] = 0;
    cli();
    char prev;
    outb(R_A,RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    // outb(2,CMOS); // not sure what should be written in port A right now (OSDEV says default is 6)
    outb(R_B, RTC); // 0x8B is the port for B, and 8 is because it wants to mask it
    prev = inb(CMOS); 
    outb(R_B, RTC); // 0x8B is the port for B, 8 because it wants to mask it
    outb(prev | 0x40 , CMOS); // ORd with 0x40 because it wants to 4th bit to be on
    enable_irq(IRQ8); // 8 is from OSDEV. which is the IRQ for RTC

    /**** TODO: change the frequency of rtc to 1024 ****/
    outb(R_A, RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    prev = inb(CMOS); // get init value of register A
    outb(R_A, RTC); //0x8A is the port for A, and 8 is because it wants to mask it
    outb((prev & RTC_MASK) | rtc_rate(MAX_FREQ), CMOS); // change rtc_rate to 1024 as a default
    sti();

return 0; //Return for success
}

/* Description: handles the rtc interrupt by incrementing all counters
* void rtc_interrupt()
* Inputs: void
* Return Value: void
* Function: increments the counters with RTC interrupts*/
void rtc_interrupt(){
    int i;
    cli();

    // increment the counter
    for(i = 0; i<NUM_TERMINALS; i++){
    rtc_counters[running_terminal->id]++;
    }

    //test_interrupts();
    outb(0x0C,RTC); // this is the offset for register C
    inb(CMOS);

    sti();

    send_eoi(IRQ8); //8 because it is the IRQ port for RTC
    return;

}




/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
* int rtc_open()
* Inputs: filename
* Return Value: uint32_t
* Function: Initializes the frequency rate to 2HZ*/
int32_t rtc_open(const uint8_t* filename){

    cli();
    /* start of critical section */
    // initializes rtc freq to 2
    running_terminal->rtc_frequency = 2;
    // reset rtc
    rtc_counters[running_terminal->id] = 0;// set the ID to be 0

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


/* Description: Checks if it is a power of two
* int isPowerOfTwo()
* inputs: int num to be checked if it is power of two
* return value:
* - 1 if it is 
* - 0 if it is not
* ref: https://www.ritambhara.in/check-if-number-is-a-power-of-2/
*/
int isPowerOfTwo(int num){
    if(num == 0){
    return 0;
    }
    while(num != 1){
    if(num % 2 != 0) {return 0;}
    num /=2;
    }
    return 1;
}


/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
* It changes the frequency of the interrupt based on the buffer pointer passed in and sets it according to algo provided above
* int rtc_write()
* Inputs: fd, buf, nbytes
* Return Value: int
* Function: Changes the rate based on the input and does 32768 >> (rate-1)*/
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes){
    // printf("ENTERS HERE");
    unsigned char temp_rate;
    temp_rate = *(unsigned char *)buf;

    // now check if it is a power of 2
    if(isPowerOfTwo(temp_rate) == 0 || temp_rate < 2){
    return -1;
    }

    cli();

    running_terminal->rtc_frequency = temp_rate;
    sti();

    return 0;

}

/* Description: The changing of interrupt follows osdev: https://wiki.osdev.org/RTC
* int rtc_write()
* Inputs: None
* Return Value: int
* Function: blocks until the next interrupt */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){

    // 1024/running_terminal->rtc_freq since we want to know if the counter has reached the counter it needs
    while(rtc_counters[running_terminal->id] <= MAX_FREQ/running_terminal->rtc_frequency){ }

    // reset counters
    rtc_counters[running_terminal->id] = 0;
    return 0;
}

/* uint32_t rtc_rate()
* Inputs: frequency
* Return Value: uint32_t
* Function: sets the rtc rate based on the frequency*/

//All return values calculated by 15 - log2(rate) + 1
int32_t rtc_rate(int32_t freq){
    switch (freq){
    case 2: return 0x0F;
    case 4: return 0x0E;
    case 8: return 0x0D;
    case 16: return 0x0C;
    case 32: return 0x0B;
    case 64: return 0x0A;
    case 128: return 0x09;
    case 256: return 0x08;
    case 512: return 0x07;
    case 1024: return 0x06;
    default: return -1;
    }
}

