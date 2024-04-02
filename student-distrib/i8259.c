/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */


/* 
 *   i8259_init
 *   DESCRIPTION: use to Initialize the 8259 PIC (setting masks, init both master and slave)
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initialize the 8259 PIC
 */
void i8259_init(void) {
    // Mask all IR pin on both PICs (+1 to get the second port of each PIC), 0xFF means mask all IR pins
    outb(0xFF, MASTER_8259_PORT + 1);
    outb(0xFF, SLAVE_8259_PORT + 1);
    master_mask = 0xFF;
    slave_mask = 0xFF;
    // Init Master PIC
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT + 1);
    outb(ICW3_MASTER, MASTER_8259_PORT + 1);
    outb(ICW4, MASTER_8259_PORT + 1);
    // Init Slave PIC
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
    outb(ICW4, SLAVE_8259_PORT + 1);
    // Mask all IR pin on both PICs, 0xFF means mask all IR pins
    outb(0xFF, MASTER_8259_PORT + 1);
    outb(0xFF, SLAVE_8259_PORT + 1);
    master_mask = 0xFF;
    slave_mask = 0xFF;
}


/* 
 *   i8259_enable_irq
 *   DESCRIPTION: use to Enable (unmask) the specified IRQ 
 *   INPUTS: irq_num -- the irq number to be enabled
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will enable the corresponding irq line
 */
void i8259_enable_irq(uint32_t irq_num) {
    // Parameter legitimacy judgment, irq_num should be 0 to (NR_IRQS - 1)
    if(irq_num>=0 && irq_num<NR_IRQS){  // if irq_num is within the range
        if(irq_num < NR_IRQS_EACH){     // if the irq line on master PIC
            outb(master_mask & (~(1 << irq_num)), MASTER_8259_PORT + 1);    // send the mask to master PIC
            master_mask = master_mask & (~(1 << irq_num));    // set mask on master_mask (mask for irq_num has been cleared)
        }else{                          // if the irq line on slave PIC
            outb(slave_mask & (~(1 << (irq_num - NR_IRQS_EACH))), SLAVE_8259_PORT + 1);    // send the mask to slave PIC
            slave_mask = slave_mask & (~(1 << (irq_num - NR_IRQS_EACH)));     // set mask on slave_mask (mask for irq_num has been cleared)
            outb(master_mask & (~(1 << ICW3_SLAVE)), MASTER_8259_PORT + 1);
            master_mask = master_mask & (~(1 << ICW3_SLAVE));
        }
    }
    
}



/* 
 *   i8259_disable_irq
 *   DESCRIPTION: use to disable (mask) the specified IRQ 
 *   INPUTS: irq_num -- the irq number to be disabled
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will disable the corresponding irq line
 */
void i8259_disable_irq(uint32_t irq_num) {
    // Parameter legitimacy judgment, irq_num should be 0 to (NR_IRQS - 1)
    if(irq_num>=0 && irq_num<NR_IRQS){  // if irq_num is within the range
        if(irq_num < NR_IRQS_EACH){     // if the irq line on master PIC
            outb(master_mask | (1 << irq_num), MASTER_8259_PORT + 1);   // send the mask to master PIC
            master_mask = master_mask | (1 << irq_num);   // set mask on master_mask (mask for irq_num has been set)
        }else{                          // if the irq line on slave PIC
            outb(slave_mask | (1 << (irq_num - NR_IRQS_EACH)), SLAVE_8259_PORT + 1);   // send the mask to slave PIC
            slave_mask = slave_mask | (1 << (irq_num - NR_IRQS_EACH));    // set mask on slave_mask (mask for irq_num has been set)
        }
    }
    
}


/* 
 *   i8259_send_eoi
 *   DESCRIPTION: Send end-of-interrupt signal for the specified IRQ 
 *   INPUTS: irq_num -- the irq that the eoi is sent to
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Send end-of-interrupt signal
 */
void i8259_send_eoi(uint32_t irq_num) {
    // Parameter legitimacy judgment, irq_num should be 0 to (NR_IRQS - 1)
    if(irq_num>=0 && irq_num<NR_IRQS){  // if irq_num is within the range
        if(irq_num < NR_IRQS_EACH){     // if the irq line on master PIC
            outb((uint8_t)(irq_num | EOI), MASTER_8259_PORT);      // send EOI to master PIC
        }else{
            outb(ICW3_SLAVE | EOI, MASTER_8259_PORT);   // send EOI to master PIC of IRQ which connected to the slave PIC, to indicate that the interrupt on the slave PIC already finished
            outb((uint8_t)((irq_num - NR_IRQS_EACH) | EOI), SLAVE_8259_PORT);  // send EOI to slave PIC 
        }
    }
}
