#include "interrupt.h"
#include "signal.h"
#include "process.h"

// flag var used to repersent whether it has an IRQ in the desc array
static int have_irq = 0;   
// define i8259_PIC, the startup and enable function are points to i8259_enable_irq, the shutdown and disable function are points to i8259_disable_irq, the act function points to the i8259_send_eoi
static PIC_chip_t i8259_PIC = {i8259_enable_irq, i8259_disable_irq, i8259_enable_irq, i8259_disable_irq, i8259_send_eoi};
// irq descriptor array
static struct irq_desc irq_desc[NR_IRQS];


/* 
 *   init_irq_desc
 *   DESCRIPTION: init irq_desc array
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the irq_desc array
 */
void init_irq_desc(){
    int i;
    // Init the i8259_PIC struct
    i8259_PIC.startup = i8259_enable_irq;
    i8259_PIC.shutdown = i8259_disable_irq;
    i8259_PIC.enable = i8259_enable_irq;
    i8259_PIC.disable = i8259_disable_irq;
    i8259_PIC.act = i8259_send_eoi;
    // Init all entries in irq_dest array
    for(i=0;i<NR_IRQS;i++){
        irq_desc[i].chip = NULL;
        irq_desc[i].depth = 0;
        irq_desc[i].flags = 0;
        irq_desc[i].handler = NULL;
        irq_desc[i].lock = SPIN_LOCK_UNLOCKED;
    }
}

/* 
 *   request_irq
 *   DESCRIPTION: use to install an interrupt handler in the irq_desc array
 *   INPUTS: irq - irq number
 *           handler - interrupt handler
 *           flags - irq flags
 *   OUTPUTS: 0 means fail, 1 means success
 *   RETURN VALUE: 0 means fail, 1 means success
 *   SIDE EFFECTS: will change irq_desc array
 */
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags){
    if(irq<0 || irq>=NR_IRQS || !handler) return 0;     // if the irq is out of range or no handler, return 0
    if(!have_irq){  // if this is the first handler
        init_irq_desc();    // init the irq_Desc
        have_irq = 1;   // set the exist flag to 1
    } 
    // set the irq_desc[irq]
    irq_desc[irq].chip = &i8259_PIC;    // link the chip
    irq_desc[irq].depth = 1;        // init depth to 1
    irq_desc[irq].handler = handler;    // link the handler
    irq_desc[irq].lock = SPIN_LOCK_UNLOCKED;    // init lock as unlock
    irq_desc[irq].flags = flags;    // set flags
    return 1;   // return 1 as success
}

/* 
 *   free_irq
 *   DESCRIPTION: use to free an irq in the irq_desc array
 *   INPUTS: irq - irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change irq_desc array
 */
void free_irq(unsigned int irq){
    if(irq<0 || irq>=NR_IRQS) return;   // if irq number out of range, return
    irq_desc[irq].chip = &i8259_PIC;    // init the chip
    irq_desc[irq].depth = 0;    // clear depth
    irq_desc[irq].handler = NULL;   // clear handler
    irq_desc[irq].lock = SPIN_LOCK_UNLOCKED;    // set unlock
    irq_desc[irq].flags = 0;    // clear flags
}

/* 
 *   disable_irq
 *   DESCRIPTION: use to disable an irq (support nested)
 *   INPUTS: irq - irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change irq_desc array, and call pic disable funticon
 */
void disable_irq(unsigned int irq){
    uint32_t flags;
    cli_and_save(flags);
    if(irq<0 || irq>=NR_IRQS){
        restore_flags(flags);
        return;   // if irq number out of range, return
    } 
    // unsigned long flag;
    // spin_lock_irqsave(&irq_desc[irq].lock, flag);
    if(irq_desc[irq].chip == NULL){     // if no pic, return
        // spin_unlock_irqrestore(&irq_desc[irq].lock, flag);
        restore_flags(flags);
        return;
    }
    irq_desc[irq].depth++;  // increase disable depth
    // printf("                           disable_irq %d, depth = %d\n", irq, irq_desc[irq].depth);
    if(irq_desc[irq].depth == 1){   // if is the first disable, call the pic disable function
        irq_desc[irq].chip->disable(irq);
    }
    // spin_unlock_irqrestore(&irq_desc[irq].lock, flag);
    restore_flags(flags);
    return;
}

/* 
 *   enable_irq
 *   DESCRIPTION: use to enable an irq (support nested)
 *   INPUTS: irq - irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change irq_desc array, and call pic enable function
 */
void enable_irq(unsigned int irq){
    uint32_t flags;
    cli_and_save(flags);
    if(irq<0 || irq>=NR_IRQS){
        restore_flags(flags);
        return;   // if irq number out of range, return
    }
    // unsigned long flag;
    // spin_lock_irqsave(&irq_desc[irq].lock, flag);
    if(irq_desc[irq].chip == NULL || irq_desc[irq].depth==0){   // if no pic or depth is 0, return
        // spin_unlock_irqrestore(&irq_desc[irq].lock, flag);
        restore_flags(flags);
        return;
    }
    irq_desc[irq].depth--;  // decrease disable depth
    // printf("                               enable_irq %d, depth = %d\n", irq, irq_desc[irq].depth);
    if(irq_desc[irq].depth == 0){   // if this is the last enable, call the pic enable funtion
        // printf("                                  enable_irq %d, depth = %d, open irq\n", irq, irq_desc[irq].depth);
        irq_desc[irq].chip->enable(irq);
    }
    // spin_unlock_irqrestore(&irq_desc[irq].lock, flag);
    restore_flags(flags);
    return;
}

/* 
 *   mask_and_act
 *   DESCRIPTION: use to mask and act the PIC
 *   INPUTS: irq - irq number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will call the act function of PIC, and mask this irq on pic
 */
int mask_and_act(unsigned int irq){
    if(irq<0 || irq>=NR_IRQS) return 0;   // if irq number out of range, return
    // spin_lock_irqsave(&irq_desc[irq].lock, flag);
    if(irq_desc[irq].chip == NULL){     // if no pic, return 0
        return 0;
    }
    disable_irq(irq);   // disable this irq
    irq_desc[irq].chip->act(irq);   // send act to pic
    return 1;   // return 1 as success
}

/* 
 *   do_IRQ
 *   DESCRIPTION: common interface for interrupt handler
 *   INPUTS: regs - a pointer point to the pt_regs struct on the stack
 *   OUTPUTS: 0 means fail, 1 means success
 *   RETURN VALUE: 0 means fail, 1 means success
 *   SIDE EFFECTS: will mask and act on PIC, and then call the handler, and then enable this irq
 */
unsigned int do_IRQ(struct pt_regs* regs){
    if(regs==NULL) return 0;
    int irq = ~regs->orig_eax;      // get irq number
    struct irq_desc * desc = irq_desc + irq;    //  find the entry
    if(irq<0 || irq>=NR_IRQS) return 0;     // if irq out of range, return 0;
    if(desc==NULL || desc->handler==NULL) return 0;     // if desc==NULL or no handler, return 0
    if(!mask_and_act(irq)) return 0;    // if unsuccessful mask and act, return 0
    if(irq==0){
        enable_irq(irq);    // enable irq
        desc->handler(irq);     // call handler
    }else{
        desc->handler(irq);     // call handler
        enable_irq(irq);    // enable irq
    }
    //
    // Signal Handle
    //
    signal_handler(current->signals, regs);

    return 1;   // return 1 as success
}
