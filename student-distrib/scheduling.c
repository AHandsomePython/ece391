#include "scheduling.h"
#include "pit.h"
#include "complie_flags.h"
//
// Sched
//
#if (MULTI_THREAD == 0)
static int pid_to_sched_unit[MAX_NUM_OF_SCHED] = {-1, -1, -1};
#endif

#if (MULTI_THREAD == 1)
static int pid_to_sched_unit[MAX_NUM_OF_SCHED + MAX_NUM_OF_THREAD] = {-1, -1, -1, -1, -1};
// thread info
static int thread_boot[MAX_NUM_OF_THREAD] = {0, 0};
static int sleep_status[MAX_NUM_OF_SCHED + MAX_NUM_OF_THREAD] = {0, 0, 0, 0, 0};
#endif

//
// Set Sched Struct
//
int32_t sched_set(PCB_t* pcb){
    if(pcb == NULL) return -1;
    int32_t index = terminal_get_index(pcb->terminal);
    if(index == -1) return -1;
    pid_to_sched_unit[index] = pcb->pid;
    return 0;
}

int32_t sched_get_index(PCB_t* pcb){
    if(pcb == NULL) return -1;
    int32_t index = terminal_get_index(pcb->terminal);
    if(index == -1) return -1;
    return index;
}

int32_t sched_get_pid(PCB_t* pcb){
    if(pcb == NULL) return -1;
    int32_t index = terminal_get_index(pcb->terminal);
    if(index == -1) return -1;
    return pid_to_sched_unit[index];
}

int32_t terminal_to_sched(terminal_t* terminal){
    if(terminal==NULL) return -1;
    int32_t index = terminal_get_index(terminal);
    if(index==-1) return -1;
    return pid_to_sched_unit[index];
}

int32_t sched_free_pid(PCB_t* pcb){
    if(pcb == NULL) return -1;
    int32_t index = terminal_get_index(pcb->terminal);
    if(index == -1) return -1;
    pid_to_sched_unit[index] = -1;
    return 0;
}

int32_t sched_get_available_pid(){
    int i=0;
    for(i=0; i<MAX_NUM_OF_SCHED; i++){
        if(pid_to_sched_unit[i]!=-1) return pid_to_sched_unit[i];
    }
    return -1;
}

int32_t sched_get_next_top(PCB_t* pcb){
    if(pcb==NULL) return -1;
    int32_t index = sched_get_index(pcb);
    if(index==-1) return -1;
    return pid_to_sched_unit[(index + 1) % MAX_NUM_OF_SCHED];
}

int32_t sched_safety_check(){
    int32_t ans = 1;
    int32_t i;
    for(i=0; i<MAX_NUM_OF_SCHED; i++){
        ans = ans && (pid_to_sched_unit[i]!=-1);
    }
    return ans;
}

#if (MULTI_THREAD == 0)
//
// Context switch interfaces:
//
int32_t context_switch_to(int32_t pid){
    PCB_t* cur_pcb = current;
    PCB_t* target_pcb;
    if(pid==-1){
        asm volatile(
            "movl %%esp, %0                         \n\t"
            "movl %%ebp, %1                         \n\t"
            "leal context_switch_return_addr, %2    \n\t"
            : "+r" (cur_pcb->esp), "+r" (cur_pcb->ebp), "+r" (cur_pcb->eip)
            :
            : "cc"
        );
        sti();
        sys_execute((uint8_t*)"shell");
    }

    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    if(is_pid_inuse(pid) != 1) return -1;

   
    target_pcb = get_pcb_from_pos(pid);

    //
    // Part 1 Set Paging
    //
    if(map_4MB_page(PROGRAM_START_VIRTUAL_ADDR, target_pcb->physical_mem_start)==-1) return -1;

    //
    // Part 2 Set TSS
    //
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_kernel_stack_from_pos(target_pcb->PCB_pos);

    //
    // Part 3 Store Current Info
    //
    asm volatile(
        "movl %%esp, %0                         \n\t"
        "movl %%ebp, %1                         \n\t"
        "leal context_switch_return_addr, %2    \n\t"
        : "+r" (cur_pcb->esp), "+r" (cur_pcb->ebp), "+r" (cur_pcb->eip)
        :
        : "cc"
    );
    
    //
    // Part 4 Context Switch
    //
    asm volatile(
        "movl %0, %%esp                 \n\t"
        "movl %1, %%ebp                 \n\t"
        "movl %2, %%eax                 \n\t"
        "jmp *%%eax                     \n\t"
        "context_switch_return_addr:    \n\t"
        :
        : "r" (target_pcb->esp), "r" (target_pcb->ebp), "r" (target_pcb->eip)
        : "eax"
    );

    return 0;
}
#endif


static volatile int num_of_sched = 0;

static volatile int cur_sched = 0;

#if (MULTI_THREAD == 0)
int32_t context_switch_rr(){
    // int32_t sched_index = sched_get_index(current);
    int32_t pid;
    if(num_of_sched<MAX_NUM_OF_SCHED){
        num_of_sched++;
        context_switch_to(-1);
    }else{
        pid = pid_to_sched_unit[cur_sched];
        cur_sched = (cur_sched + 1) % MAX_NUM_OF_SCHED;
        if(pid==-1) return -1;
        // kprintf("pid %d\n", pid);
        context_switch_to(pid);
    }
    return 0;
}
#endif

#if (MULTI_THREAD == 1)
//
// Context switch interfaces:
//
int32_t context_switch_to(int32_t pid, int32_t tflag){
    PCB_t* cur_pcb = current;
    PCB_t* target_pcb = get_pcb_from_pos(pid);
    uint32_t flags;
    if(pid==-1){
        asm volatile(
            "movl %%esp, %0                         \n\t"
            "movl %%ebp, %1                         \n\t"
            "leal context_switch_return_addr, %2    \n\t"
            : "+r" (cur_pcb->esp), "+r" (cur_pcb->ebp), "+r" (cur_pcb->eip)
            :
            : "cc"
        );
        sti();
        sys_execute((uint8_t*)"shell");
    }

    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    if(is_pid_inuse(pid) != 1) return -1;

   
    // target_pcb = get_pcb_from_pos(pid);

    //
    // Part 1 Set Paging
    //
    if(map_4MB_page(PROGRAM_START_VIRTUAL_ADDR, target_pcb->physical_mem_start)==-1) return -1;
    map_vid_page(pid);
    
    //
    // Part 2 Set TSS
    //
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_kernel_stack_from_pos(target_pcb->PCB_pos);

    //
    // Part 3 Store Current Info
    //
    asm volatile(
        "movl %%esp, %0                         \n\t"
        "movl %%ebp, %1                         \n\t"
        "leal context_switch_return_addr, %2    \n\t"
        : "+r" (cur_pcb->esp), "+r" (cur_pcb->ebp), "+r" (cur_pcb->eip)
        :
        : "cc"
    );

    if(tflag == 1){
        cli_and_save(flags);
        flags |= 0x0200;
        // asm volatile(
        //     "movl %0, %%ebp     \n\t"
        //     : 
        //     : "r" (target_pcb->ebp)
        //     : "eax"
        // );
        asm volatile(
            "pushl %0           \n\t"
            "pushl %1           \n\t"   // push user program esp
            "pushl %4           \n\t"
            "pushl %2           \n\t"   // push code segment information
            "pushl %3           \n\t"   // push user program eip
            "iret               \n\t"
            :
            : "r" ((uint32_t)USER_DS), "r" (target_pcb->esp), "r" ((uint32_t)USER_CS), "r" (target_pcb->eip), "r" (flags)
            : "eax"
        );
    }

    //
    // Part 4 Context Switch
    //
    asm volatile(
        "movl %0, %%esp                 \n\t"
        "movl %1, %%ebp                 \n\t"
        "movl %2, %%eax                 \n\t"
        "jmp *%%eax                     \n\t"
        "context_switch_return_addr:    \n\t"
        :
        : "r" (target_pcb->esp), "r" (target_pcb->ebp), "r" (target_pcb->eip)
        : "eax"
    );

    return 0;
}


static int cur_thread = 0;
static int sched_thread_flag = 0;
int32_t context_switch_rr(){
    // int32_t sched_index = sched_get_index(current);
    int32_t pid;
    int32_t i;
    if(num_of_sched<MAX_NUM_OF_SCHED){
        num_of_sched++;
        context_switch_to(-1, 0);
    }else{
        if(sched_thread_flag == 1){
            int32_t cur = cur_thread;
            for(i=0; i<MAX_NUM_OF_THREAD; i++){
                pid = pid_to_sched_unit[MAX_NUM_OF_SCHED + cur_thread];
                cur = cur_thread;
                cur_thread = (cur_thread + 1) % MAX_NUM_OF_THREAD;
                if(cur_thread==0){
                    sched_thread_flag = 0;
                    break;
                }
                if(pid==-1) continue;
                break;
            }
            if(pid!=-1 && sleep_status[MAX_NUM_OF_SCHED + cur]==0){
                if(thread_boot[cur]==0){
                    thread_boot[cur] = 1;
                    context_switch_to(pid, 1);
                }else{
                    context_switch_to(pid, 0);
                }
            }else{
                return -1;
            }
        }else{
            pid = pid_to_sched_unit[cur_sched];
            cur_sched = (cur_sched + 1) % MAX_NUM_OF_SCHED;
            if(cur_sched==0){
                sched_thread_flag = 1;
            }
            if(pid!=-1 && sleep_status[cur_sched]==0){
                context_switch_to(pid, 0);
            }else{
                return -1;
            }
            // kprintf("pid %d\n", pid);
            
        }
    }
    return 0;
}

void schedule(){
    context_switch_rr();
}

//
// Thread interfaces:   目前还有一个问题，就是需要在PCB里面加一个字段记录地址空间，屏蔽中断！！！
//
#define USER_STACK_SIZE 0x00008000

int32_t create_thread(void (*func)()){
    if(func==NULL) return -1;
    if(current->thread_num>=MAX_THREAD_NUM_FOR_EACH) return -1;

    uint32_t flags;
    cli_and_save(flags);

    //
    // Part 1 Set PCB
    //
    int32_t pcb_pos = get_and_set_new_PCB_pos();
    if(pcb_pos==-1){
        restore_flags(flags);
        return -1;
    }
    PCB_t* pcb = get_pcb_from_pos(pcb_pos);
    PCB_init(pcb);
    pcb->pid = pcb_pos;
    pcb->PCB_pos = pcb_pos;
    pcb->status = 1;
    pcb->argc = 0;
    pcb->parent = current;
    pcb->eip = (uint32_t)func;
    pcb->shell_flag = 0;
    pcb->window = NULL;
    pcb->terminal = pcb->parent->terminal;
    pcb->physical_mem_start = pcb->parent->physical_mem_start;
    pcb->thread_flag = 1;
    pcb_signal_init(pcb->PCB_pos);
    

    //
    // Part 2 Set thread info
    //
    int i;
    PCB_t* cur_pcb = current;
    for(i=0; i<MAX_THREAD_NUM_FOR_EACH; i++){
        if(cur_pcb->threads[i]==-1){
            cur_pcb->threads[i] = pcb->pid;
            break;
        }
    }
    if(i==MAX_THREAD_NUM_FOR_EACH){
        restore_flags(flags);
        return -1;
    }
    cur_pcb->thread_num++;
    uint32_t stack_index = i + 1;
    pcb->esp = USER_STACK_ADDR - stack_index * USER_STACK_SIZE;
    pcb->ebp = USER_STACK_ADDR - stack_index * USER_STACK_SIZE;

    for(i=0; i<MAX_NUM_OF_THREAD; i++){
        if(pid_to_sched_unit[MAX_NUM_OF_SCHED + i] == -1){
            pid_to_sched_unit[MAX_NUM_OF_SCHED + i] = pcb->pid;
            thread_boot[i] = 0;
            pcb->global_thread_index = i;
            break;
        }
    }
    if(i==MAX_NUM_OF_THREAD){
        restore_flags(flags);
        return -1;
    }

    restore_flags(flags);
    return 0;
}

// if pid==0, means killing itself
int32_t kill_thread(int32_t pid){
    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    PCB_t* pcb;
    if(pid==0){
        pcb = current;       // pcb pointer
        if(current->thread_flag!=1) return -1;
    }else{
        pcb = get_pcb_from_pos(pid);
        if(is_pid_inuse(pid)!=1 || get_pcb_from_pos(pid)->thread_flag!=1) return -1;
    }
    PCB_t* parent = pcb->parent;

    uint32_t flags;
    cli_and_save(flags);

    //
    // Part 1 free PCB
    //
    int i;
    for(i=0; i<FILEARR_SIZE; i++){
        if(pcb->filearr[i].flags & FILEDESC_FLAG_INUSE){
            if(pcb->filearr[i].ops!=NULL) pcb->filearr[i].ops->close(i);
        }
    }
    if(pcb->window!=NULL) GUI_window_free(pcb->window);
    pcb->window = NULL;
    free_PCB_pos(pcb->PCB_pos);     // free current pid in the process id array
    for(i=0; i<FILEARR_SIZE; i++){  //close the relavant files
        if(pcb->filearr[i].flags == FILEDESC_FLAG_INUSE){
            pcb->filearr[i].ops->close(i);
        }
        pcb->filearr[i].file_position = 0;
        pcb->filearr[i].flags = FILEDESC_FLAG_FREE;
        pcb->filearr[i].inode_index = -1;
        pcb->filearr[i].ops = NULL;    //reset the variable to initial value
    }
    thread_boot[pcb->global_thread_index] = 0;
    pid_to_sched_unit[MAX_NUM_OF_SCHED + pcb->global_thread_index] = -1;
    sleep_status[MAX_NUM_OF_SCHED + pcb->global_thread_index] = 0;

    //
    // Part 2 update parent info
    //
    parent->thread_num--;
    for(i=0; i<MAX_THREAD_NUM_FOR_EACH; i++){
        if(parent->threads[i]==pcb->pid){
            parent->threads[i] = -1;
            break;
        }
    }

    if(pid==0){
        context_switch_to(0, 0);
    }
    restore_flags(flags);
    return 0;
}

int32_t process_sleep(int32_t pid){
    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    uint32_t flags;
    cli_and_save(flags);
    PCB_t* pcb = get_pcb_from_pos(pid);
    int32_t sched_index;
    if(pcb->thread_flag){
        sched_index = MAX_NUM_OF_SCHED + pcb->global_thread_index;
    }else{
        sched_index = sched_get_index(get_pcb_from_pos(pid));
    }
    sleep_status[sched_index] = 1;
    if(pid==current->pid){
        schedule();
    }
    restore_flags(flags);
    return 0;
}

int32_t process_awake(int32_t pid){
    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    uint32_t flags;
    cli_and_save(flags);
    PCB_t* pcb = get_pcb_from_pos(pid);
    int32_t sched_index;
    if(pcb->thread_flag){
        sched_index = MAX_NUM_OF_SCHED + pcb->global_thread_index;
    }else{
        sched_index = sched_get_index(get_pcb_from_pos(pid));
    }
    sleep_status[sched_index] = 0;
    restore_flags(flags);
    return 0;
}

#endif
