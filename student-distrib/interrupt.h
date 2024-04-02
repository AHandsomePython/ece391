#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "spinlock.h"
#include "i8259.h"
#include "lib.h"

// struct used for pt_regs which will pushed on the stack when interrupt happens
struct pt_regs{
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t orig_eax;
    uint32_t dummy;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
}__attribute__ ((packed));

// an level of inderection, which is a interface to PIC chip
typedef struct PIC_chip{
    void (*startup)(unsigned int irq);      // PIC startup function
    void (*shutdown)(unsigned int irq);     // PIC shutdown function
    void (*enable)(unsigned int irq);       // PIC enable function
    void (*disable)(unsigned int irq);      // PIC disable function
    void (*act)(unsigned int irq);          // PIC act function
}PIC_chip_t;

// define irq_handler_t is a function pointer type which points a function with unsigned int input and int returt value
typedef int (*irq_handler_t)(unsigned int);

// define a struct irq_desc as the IRQ descriptor
struct irq_desc{
    unsigned int flags;     // flags
    unsigned int depth;     // disable depth
    PIC_chip_t* chip;       // points to the PIC chip
    spinlock_t lock;        // descriptor lock
    irq_handler_t handler;  // points to interrupt handler
};

void init_irq_desc();   // init irq desc array
int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags);  // request an irq
void free_irq(unsigned int irq);        // free an irq
void disable_irq(unsigned int irq);     // disable certain irq
void enable_irq(unsigned int irq);      // enable certain irq
int mask_and_act(unsigned int irq);     // mask the irq on pic and act

unsigned int do_IRQ(struct pt_regs* regs);      // do_IRQ function used as a general function to handle interrupts

#endif
