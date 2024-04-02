#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "paging.h"
#define get_pcb_from_pos(pcb_pos) ((PCB_t*)((PAGE_4MB_VAL << 1) - (pcb_pos + 1) * (PAGE_4KB_VAL << 1)))
#define get_pcb_index(pcb) (((uint32_t)get_pcb_from_pos(0) - (uint32_t)pcb) >> (PAGE_4KB_OFF + 1))
#define get_kernel_stack_from_pos(pcb_pos) ((uint32_t)(((uint32_t)get_pcb_from_pos(pcb_pos)) + (PAGE_4KB_VAL << 1) - 4))

#define EXEC_MAX_NUM_SHELL 3

#define USER_STACK_ADDR (0x08400000 - 4)

int32_t get_num_of_shell();
uint32_t set_virtual_memory(uint32_t pcb_pos);

// User Level Wrapper
extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn (void);
extern int32_t sigsend(int32_t signum, int32_t pid);

// Kernel Level System Call

//syetem call: terminate a process
int32_t sys_halt (uint8_t status);

//system call: load, set and execute a new program
int32_t sys_execute (const uint8_t* command);

//system call： read data from the keyboard, a file, RTC, and directory
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);

//system call: write data to the terminal & RTC
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);

//system call: open a file according to its file type and return a file descriptor
int32_t sys_open (const uint8_t* filename);

//system call:  close a file and release the fd
int32_t sys_close (int32_t fd);

//system call: returns the argument saved before
int32_t sys_getargs (uint8_t* buf, int32_t nbytes);

//system call: print "sys_vidmap"
int32_t sys_vidmap (uint8_t** screen_start);

//system call: print "sys_set_handler"
int32_t sys_set_handler(int32_t signum, void* handler_address);

//system call: print "sys_sigreturn"
int32_t sys_sigreturn (void);

// syscall #11
int32_t sys_sigsend(int32_t signum, int32_t pid);

// syscall #12
void* sys_malloc(uint32_t size);

// syscall #13
void sys_free(void* ptr);

// syscall #14
int32_t sys_thread_create(void* func);

// syscall #15
int32_t sys_thread_kill(int32_t pid);

#endif
