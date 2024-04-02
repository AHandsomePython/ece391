#include "process.h"
#include "lib.h"
#include "x86_desc.h"
#include "text_terminal.h"
#include "keyboard.h"
#include "terminal.h"
#include "paging.h"
#include "syscall.h"

static file_ops_t terminal_ops = {terminal_read, terminal_write, terminal_open, terminal_close};
static file_ops_t keyboard_ops = {keyboard_read, keyboard_write, keyboard_open, keyboard_close};

// static uint32_t pid = 0;

static int32_t process_inuse[MAX_PROCESS_NUM] = {0, 0, 0, 0, 0, 0};     // pid -> inuse

/* 
 * pid_alloc
 *   DESCRIPTION: allocate a pid for the process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the process id,  if successful
 *                 -1 if fail to allocate
 *   SIDE EFFECTS: none
 */
int32_t pid_alloc(){
    int i;
    for(i=0;i<MAX_PROCESS_NUM;i++){
        if(process_inuse[i] == 0) return i;  //check next available id, return it
    }
    return -1;
}

int32_t is_any_process_now(){
    int i;
    for(i=0; i<MAX_PROCESS_NUM; i++){
        if(process_inuse[i]==1) return 1;
    }
    return 0;
}

int32_t is_pid_inuse(int32_t pid){
    if(pid<0 || pid>MAX_PROCESS_NUM) return -1;
    return process_inuse[pid];
}

int32_t is_PCB_legal(PCB_t* pcb){
    if(get_num_of_shell()<=0) return 0;
    return 1;
}

/* 
 * current_thread_PCB
 *   DESCRIPTION: point to the current process
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the pointer to the current process
 *   SIDE EFFECTS: none
 */
inline PCB_t* current_thread_PCB()
{
    uint32_t ptr = 0;
    asm volatile(
        "movl $-8192, %0        \n\t"
        "andl %%esp, %0          \n\t"   
        : "+r" (ptr)
        : 
        : "cc"
    );
    return (PCB_t*)ptr;
}


/* 
 * PCB_init
 *   DESCRIPTION: initialize the pcb. Assign a initial value to each variable in the pcb
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success
 *                 -1 if failure
 *   SIDE EFFECTS: none
 */
int32_t PCB_init(PCB_t* pcb){
    if(pcb == NULL) return -1;
    pcb->pid = pid_alloc();     // assign a available pid
    pcb->status = PCB_STATUS_READY;       //update the pcb's status
    pcb->parent = NULL;         //initialize the pcb's parent
    pcb->argc = 0;
    pcb->signals = NULL;
    pcb->shell_flag = 0;
    pcb->terminal = NULL;
    pcb->window = NULL;
    int i;
    // init stdin
    pcb->filearr[0].file_position = 0;
    pcb->filearr[0].flags = FILEDESC_FLAG_INUSE;
    pcb->filearr[0].inode_index = -1;
    pcb->filearr[0].ops = &keyboard_ops;
    // init stdout
    pcb->filearr[1].file_position = 0;
    pcb->filearr[1].flags = FILEDESC_FLAG_INUSE;
    pcb->filearr[1].inode_index = -1;
    pcb->filearr[1].ops = &terminal_ops;
    // init memory map
    pcb->physical_mem_start = 0;
    // init other file descriptor
    for(i=2;i<FILEARR_SIZE;i++){
        pcb->filearr[i].file_position = 0;
        pcb->filearr[i].flags = FILEDESC_FLAG_FREE;
        pcb->filearr[i].inode_index = -1;
        pcb->filearr[i].ops = NULL;
    }
    // init threads
    pcb->thread_flag = 0;
    pcb->thread_num = 0;
    pcb->global_thread_index = 0;
    for(i=0;i<MAX_THREAD_NUM_FOR_EACH; i++){
        pcb->threads[i] = -1;
    }
    return 0;
}


/* 
 * get_and_set_new_PCB_pos
 *   DESCRIPTION: update the value in array process_inuse
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pid if success
 *                 -1 if failure
 *   SIDE EFFECTS: none
 */
int32_t get_and_set_new_PCB_pos(){
    int i;
    for(i=0; i<MAX_PROCESS_NUM; i++){
        if(process_inuse[i]==0){
            process_inuse[i] = 1;   // set the status to used
            return i;
        }
    }
    return -1;
}


/* 
 * free_PCB_pos
 *   DESCRIPTION: set the value in array process_inuse to unuse
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success
 *                 -1 if failure
 *   SIDE EFFECTS: none
 */
int32_t free_PCB_pos(uint32_t pcb_pos){
    if(pcb_pos>=MAX_PROCESS_NUM) return -1;
    process_inuse[pcb_pos] = 0;   // set the status to unuse
    return 0;
}

//
// Front process management
//
static PCB_t* top_process = NULL;

int32_t top_process_set(int32_t pid){
    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    top_process = get_pcb_from_pos(pid);
    if (is_modex()==0){
        int index = terminal_get_index(top_process->terminal);
        vga_show_set(index);
    }
    return 0;
}

PCB_t* top_process_get(){
    return top_process;
}
