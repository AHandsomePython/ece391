#include "process.h"
#include "lib.h"
#include "interrupt.h"
#include "exception.h"
#include "scheduling.h"
#include "complie_flags.h"

#define SIG_RETURN_SYS_CALL_INDEX 10

#define get_bit(val, index) ((val>>index) & 0x1)
#define set_bit(val, index) (val = (val | (1<<index)))
#define free_bit(val, index) (val = (val & (~(1<<index))))



void kill_the_task(int signum){
    sys_halt(EXCEPTION_HALT_STATUS);
    return;
}

void ignore_sig(int signum){
    return;
}

static signal_handler_t sig_defult_handlers[SIGNAL_NUM] = {kill_the_task, kill_the_task, kill_the_task, ignore_sig, ignore_sig};
static signal_t pcb_signal[MAX_PROCESS_NUM];



int32_t pcb_signal_init(int32_t pcb_pos){
    if(pcb_pos==-1) return -1;
    signal_t* ptr = &pcb_signal[pcb_pos];
    ptr->mask = 0;
    ptr->pending = 0;
    ptr->oldmask = 0;
    ptr->userdef = 0;
    ptr->hw_context_esp = 0;
    int i;
    for(i=0; i<SIGNAL_NUM; i++){
        ptr->handlers[i] = sig_defult_handlers[i];
    }
    get_pcb_from_pos(pcb_pos)->signals = ptr;
    return 0;
}

int32_t user_set_signal(signal_t* signals, int signum, signal_handler_t handler){
    if(signals==NULL || signum<0 || signum>=SIGNAL_NUM) return -1;
    if(handler==NULL){
        free_bit(signals->userdef, signum);
        signals->handlers[signum] = sig_defult_handlers[signum];
    }else{
        set_bit(signals->userdef, signum);
        signals->handlers[signum] = handler;
    }
    return 0;
}



int32_t run_user_sig_handler(signal_t* signals, struct pt_regs* regs, int signum){
    uint32_t esp = regs->esp;
    uint32_t tag1, tag2;

    //
    // Part 1 Set User Stack
    //
    asm volatile(
        "jmp run_user_sig_handler_02        \n\t"
        ".align 4                           \n\t"
        "run_user_sig_handler_01:           \n\t"
        "movl $10, %%eax                    \n\t"
        "int $0x80                          \n\t"
        ".align 4                           \n\t"
        "run_user_sig_handler_02:           \n\t"
        "leal run_user_sig_handler_01, %0   \n\t"
        "leal run_user_sig_handler_02, %1   \n\t"
        : "+r" (tag1), "+r" (tag2)
        :
        : "eax"
    );
    uint32_t code_size = tag2 - tag1;
    // push code to the stack
    esp -= code_size;
    uint32_t ret_addr = esp;
    memcpy((void*)esp, (void*)tag1, code_size);
    // push hw context to the stack
    esp -= sizeof(struct pt_regs);
    memcpy((void*)esp, regs, sizeof(struct pt_regs));
    signals->hw_context_esp = esp;
    // push signal num to the stack
    esp -= sizeof(uint32_t);
    *((uint32_t*)esp) = signum;
    // push return addr to the stack
    esp -= sizeof(uint32_t);
    *((uint32_t*)esp) = ret_addr;

    //
    // Part 2 Change Context
    //
    regs->esp = esp;
    regs->eip = (uint32_t)signals->handlers[signum];
    return 0;
}



int32_t signal_handler(signal_t* signals, struct pt_regs* regs){
    if(signals == NULL || regs == NULL) return -1;
    if(regs->cs!=USER_CS && regs->ds!=USER_DS){
        // kprintf("SIGNAL HANDLE: NOT RET TO USER, ERR\n");
        return -1;
    } 
    int i;
    for(i=0; i<SIGNAL_NUM; i++){
        if(get_bit(signals->pending, i)){
            free_bit(signals->pending, i);
            if(get_bit(signals->mask, i)){
                continue;
            }
            if(get_bit(signals->userdef, i)){
                signals->oldmask = signals->mask;
                signals->mask = -1;
                run_user_sig_handler(signals, regs, i);
            }else{
                signals->oldmask = signals->mask;
                signals->mask = -1;
                signals->handlers[i](i);
                signals->mask = signals->oldmask;
                signals->oldmask = 0;
            }
            break;
        }
    }
    return 0;
}

int32_t user_signal_return(signal_t* signals){
    if(signals==NULL || signals->hw_context_esp==0) return -1;
    cli();
    uint32_t regs = signals->hw_context_esp;
    signals->mask = signals->oldmask;
    signals->oldmask = 0;
    
    // 以下内容对pt_regs的结构有依赖，请注意！
    asm volatile(
        "pushl 16*4(%0)                     \n\t"
        "pushl 15*4(%0)                     \n\t"
        "pushl 14*4(%0)                     \n\t"
        "pushl 13*4(%0)                     \n\t"
        "pushl 12*4(%0)                     \n\t"
        "pushl 11*4(%0)                     \n\t"
        "pushl 10*4(%0)                     \n\t"
        "pushl 9*4(%0)                      \n\t"
        "pushl 8*4(%0)                      \n\t"
        "pushl 7*4(%0)                      \n\t"
        "pushl 6*4(%0)                      \n\t"
        "pushl 5*4(%0)                      \n\t"
        "pushl 4*4(%0)                      \n\t"
        "pushl 3*4(%0)                      \n\t"
        "pushl 2*4(%0)                      \n\t"
        "pushl 1*4(%0)                      \n\t"
        "pushl 0*4(%0)                      \n\t"
        "popl  %%ebx                        \n\t"
        "popl  %%ecx                        \n\t"
        "popl  %%edx                        \n\t"
        "popl  %%esi                        \n\t"
        "popl  %%edi                        \n\t"
        "popl  %%ebp                        \n\t"
        "popl  %%eax                        \n\t"
        "popl  %%ds                         \n\t"
        "popl  %%es                         \n\t"
        "popl  %%fs                         \n\t"
        "addl  $8, %%esp                    \n\t"
        "iret                               \n\t"
        : 
        : "r" (regs)
        : "eax"
    );
    return 0;
}

int32_t kernal_send_signal(int signum, int32_t target_pid){
    if(target_pid<0 || target_pid>=MAX_PROCESS_NUM || signum<0 || signum>=SIGNAL_NUM) return -1;
    signal_t* signals = get_pcb_from_pos(target_pid)->signals;
    set_bit(signals->pending, signum);
    return 0;
}

int32_t syscall_ret_sig_handler(struct pt_regs* regs){
    if(current->signals==NULL || regs==NULL) return -1;
    return signal_handler(current->signals, regs);
}
