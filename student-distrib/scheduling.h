#ifndef SCHEDULING_H
#define SCHEDULING_H
#include "lib.h"
#include "process.h"
#include "syscall.h"
#include "interrupt.h"
#include "complie_flags.h"

#define MAX_NUM_OF_SCHED TERMINAL_NUM
#define MAX_NUM_OF_THREAD 2

#define CONTEXT_FLAG_NORMAL 0
#define CONTEXT_FLAG_EXEC 1

//
// Set Sched Struct
//
int32_t sched_set(PCB_t* pcb);
int32_t sched_get_index(PCB_t* pcb);
int32_t sched_get_pid(PCB_t* pcb);
int32_t sched_free_pid(PCB_t* pcb);
int32_t sched_get_available_pid();
int32_t sched_safety_check();
int32_t sched_get_next_top(PCB_t* pcb);
int32_t terminal_to_sched(terminal_t* terminal);

//
// Context switch interfaces:
//
#if (MULTI_THREAD == 0)
int32_t context_switch_to(int32_t pid);
#endif
#if (MULTI_THREAD == 1)
int32_t context_switch_to(int32_t pid, int32_t tflag);
#endif
int32_t context_switch_rr();

//
// Thread syscalls:
//
int32_t create_thread(void (*func)());
int32_t kill_thread(int32_t pid);

#endif
