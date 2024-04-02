#ifndef SIGNAL_H
#define SIGNAL_H
#include "lib.h"
#include "interrupt.h"

#define SIGNAL_NUM 5

typedef enum{
    DIV_ZERO = 0,
    SEGFAULT = 1,
    INTERRUPT = 2,
    ALARM = 3,
    USER1 = 4
}signal_index_t;

typedef void (*signal_handler_t) (int);

typedef struct signal{
    uint32_t mask;
    uint32_t pending;
    uint32_t userdef;
    uint32_t oldmask;
    uint32_t hw_context_esp;
    signal_handler_t handlers[SIGNAL_NUM];
}signal_t;

int32_t pcb_signal_init(int32_t pcb_pos);
int32_t user_set_signal(signal_t* signals, int signum, signal_handler_t handler);
int32_t signal_handler(signal_t* signals, struct pt_regs* regs);
int32_t user_signal_return(signal_t* signals);
int32_t kernal_send_signal(int signum, int32_t target_pid);

int32_t syscall_ret_sig_handler(struct pt_regs* regs);

#endif
