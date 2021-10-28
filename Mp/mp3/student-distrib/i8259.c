/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
//follows lecture slide 9 logic
/*DESCRIPTION 
*   this function initializes the PIC
* INPUT = NONE
* OUTPUT = NONE
* SIDE EFFECTS= Initializes the PIC with the different control words and connects the Slave port to IRQ2 on the master
*/

void i8259_init(void) {
    //TODO: need to add a lock
    cli();
    master_mask = 0xFF; // mask it to all 1111 1111
    slave_mask = 0xFF; // mask it to all 1111 1111

    // outb(master_mask, MASTER_8259_PORT + 0x01);
    // outb(slave_mask, SLAVE_8259_PORT + 0x01);

    // NEED TO DO IT IN ORDER
    // (referencing the student notes give in the course notes). 
    // it says that the remaning ICWs are written to the seocnd port of each PIC

    
    // initialize master and slave alternating
    outb(ICW1, MASTER_8259_PORT);
    outb (ICW2_MASTER, MASTER_8259_PORT + 0x01);    // it adds 0x01 because it wants to write to the second port (DATA)
    outb (ICW3_MASTER, MASTER_8259_PORT + 0x01);    // it adds 0x01 because it wants to write to the second port (DATA)
    outb (ICW4, MASTER_8259_PORT + 0x01);           // it adds 0x01 because it wants to write to the second port (DATA)
    outb (ICW1, SLAVE_8259_PORT);
    outb (ICW2_SLAVE, SLAVE_8259_PORT + 0x01);      // it adds 0x01 because it wants to write to the second port (DATA)
    outb (ICW3_SLAVE, SLAVE_8259_PORT + 0x01);      // it adds 0x01 because it wants to write to the second port (DATA)
    outb (ICW4, SLAVE_8259_PORT + 0x01);            // it adds 0x01 because it wants to write to the second port (DATA)
    
    outb(master_mask, MASTER_8259_PORT + 0x01);     // it adds 0x01 because it wants to write to the second port (DATA)
    outb(slave_mask, SLAVE_8259_PORT + 0x01);       // it adds 0x01 because it wants to write to the second port (DATA)

    enable_irq(2);  //enable irq for slave (2nd port)
    //unmask the interrupts (use outb)
    
    //TODO:need to add an unlock
    sti();
}

/* Enable (unmask) the specified IRQ */
//follows OSDEV logic
//credit is given to https://urldefense.com/v3/__https://wiki.osdev.org/8259_PIC__;!!DZ3fjg!tLjUW3Es0CZE6PP_sc5By2r1dJFPLxm86CGkfF1n3LM3OYV5RhKttcWbSYh8DHqo$ 
/*DESCRIPTION 
*   this function enables interrupts that is sent to the PIC
* INPUT = interrupt request number
* OUTPUT = NONE
* SIDE EFFECTS= should essentially unmask the interrupt
*/
void enable_irq(uint32_t irq_num) {
    uint16_t port;
    //uint8_t value;

    //boundary conditions
    if (irq_num > 15 || irq_num <0){ // between 0 and 15 because there are 16 IRQs
        printf(" ======= garbage input irq %d invalid ==========", irq_num);
        return;
    }
    if(irq_num < 8){ //less than 8 means that it is on the master
        port = MASTER_8259_PORT + 0x01; // adds 0x01 because it wants to second port
        master_mask = master_mask & ~(1 << irq_num); //setting the bits, it is used with 0xFF because it needs to be ANDED
        outb(master_mask, port);
    }
    else{
        port = SLAVE_8259_PORT + 0x01;     // it adds 0x01 because it wants to write to the second port (DATA)
        irq_num -= 8; // 8 because it originally over 8, and there is 8 ports in the master
        slave_mask = slave_mask & ~(1 << irq_num); //setting the bits, it is used with 0xFF because it needs to be ANDED (1 is 0x01), so it'll change when shifted
        outb(slave_mask, port);
    }
}

/* Disable (mask) the specified IRQ */
//follows OSDEV logic
//credit is given to https://urldefense.com/v3/__https://wiki.osdev.org/8259_PIC__;!!DZ3fjg!tLjUW3Es0CZE6PP_sc5By2r1dJFPLxm86CGkfF1n3LM3OYV5RhKttcWbSYh8DHqo$ 
/*DESCRIPTION 
*   this function disables interrupts
* INPUT = the interrupt request number
* OUTPUT = NONE
* SIDE EFFECTS= should disable the interrupts
*/
void disable_irq(uint32_t irq_num) {
    uint16_t port;
    //uint8_t value;

    //boundary conditions
    if (irq_num > 15 || irq_num <0){ // between 0 and 15 because there are only 16 IRQs on the master and slave PIC
        printf(" ======== garbage input irq %d invalid ==========", irq_num);
        return;
    }
    if(irq_num < 8){
        port = MASTER_8259_PORT + 0x01; // adds 0x01 because it wants to second port
        master_mask = master_mask | (1 << irq_num); //setting the bits, it is used with 0xFF because it needs to be ORd (1 is 0x01), so it'll change when shifted
        outb (master_mask, port);
    }
    else{
        port = SLAVE_8259_PORT + 0x01; // adds 0x01 because it wants to second port
        irq_num -= 8; // 8 because it originally over 8, and there is 8 ports in the master
        slave_mask = slave_mask | (1 << irq_num); //setting the bits, it is used with 0xFF because it needs to be ORd (1 is 0x01), so it'll change when shifted
        outb (slave_mask, port);
    }
    
}

/* Send end-of-interrupt signal for the specified IRQ */
// followed OSDEV for the general logic, if it is from the slave, needs to send to both master and slave
//credit is given to https://urldefense.com/v3/__https://wiki.osdev.org/8259_PIC__;!!DZ3fjg!tLjUW3Es0CZE6PP_sc5By2r1dJFPLxm86CGkfF1n3LM3OYV5RhKttcWbSYh8DHqo$ 
/*DESCRIPTION 
*   this function sends an EOI signal when the interrupt is done
* INPUT = the interrupt request number
* OUTPUT = NONE
* SIDE EFFECTS= sends a EOI, if it is from the slave, nees to be from both the slave and Master
*/
void send_eoi(uint32_t irq_num) {
    unsigned char end_of_intr;
    end_of_intr = EOI | (irq_num);
    if (irq_num >= 8){// if it is in slave. 8 is because there is only 8 slots on the master PIC
        end_of_intr = EOI | (irq_num - 8); // 8 because it wants to offset from the master
        outb(end_of_intr,SLAVE_8259_PORT);
        outb((EOI | 2 ), MASTER_8259_PORT); // 2 because slave is mapped to second
    }
    else{
        outb(end_of_intr,MASTER_8259_PORT);
    }
}
