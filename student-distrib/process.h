#ifndef PROCESS_H
#define PROCESS_H

#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "syscall.h"
#include "text_terminal.h"
#include "GUI/window.h"
#include "terminal.h"
#include "signal.h"

#define FILEARR_SIZE 8
#define PCB_STATUS_READY 1
#define MAX_PROCESS_NUM 6
#define PCB_SHELL_FLAG 0x8
#define ARGV_MAX_LEN 100
#define MAX_THREAD_NUM_FOR_EACH 2

#define current current_thread_PCB()
#define top_pcb top_process_get()

//allocate a pid for the process
int32_t pid_alloc();

typedef struct PCB{
    uint32_t pid;   // 0 to 5 
    filed_t filearr[FILEARR_SIZE]; // fd 
    uint32_t status;
    uint32_t PCB_pos;
    uint32_t argc;  // get arg ??? 
    uint8_t argv[ARGV_MAX_LEN + 1];
    struct PCB* parent; // 
    terminal_t* terminal;
    GUI_window_t* window;
    // TSS
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    // Memory Map
    uint32_t physical_mem_start;
    // Shell
    uint32_t shell_flag;
    // Signal
    signal_t* signals;
    // Thread
    int32_t thread_flag;
    int32_t thread_num;
    int32_t global_thread_index;
    int32_t threads[MAX_THREAD_NUM_FOR_EACH];
}PCB_t;

//point to the current process
inline PCB_t* current_thread_PCB();

//initialize the pcb. Assign a initial value to each variable in the pcb
int32_t PCB_init(PCB_t* pcb);

//update the value in array process_inuse
int32_t get_and_set_new_PCB_pos();

//set the value in array process_inuse to unuse
int32_t free_PCB_pos(uint32_t pcb_pos);

int32_t is_any_process_now();

int32_t is_PCB_legal(PCB_t* pcb);

int32_t is_pid_inuse(int32_t pid);

//
// Front process management
//
int32_t top_process_set(int32_t pid);
PCB_t* top_process_get();

#endif
