
#define ASM     1

#include "x86_desc.h"


.text

.global system_call_link, syscall_ret_sig_handler
.global sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn, sys_sigsend
.global sys_malloc, sys_free, sys_thread_create, sys_thread_kill
.global halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn, sigsend, malloc, free, thread_create, thread_kill 

# system call function (Kernel Level)
sys_call_table: 
    .long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn, sys_sigsend, sys_malloc, sys_free, sys_thread_create, sys_thread_kill


# system call linkage (Kernel Level)
system_call_link:
    # store all regs
    pushl $0    # push dummy
    pushl $0x80     # push syscall magic
    pushl %fs
    pushl %es
    pushl %ds
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    # check the arg
    cmpl $1, %eax
    jl system_call_fault
    cmpl $15, %eax
    jg system_call_fault
    # call the func
    subl $1, %eax
    call *sys_call_table(, %eax, 4)

    pushl %eax
    movl %esp, %eax
    addl $4, %eax
    pushl %eax
    call syscall_ret_sig_handler
    addl $4, %esp
    popl %eax
    jmp system_call_done

system_call_fault:
    movl $-1, %eax
system_call_done:
    # restore all regs
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    addl $4, %esp
    popl %ds
    popl %es
    popl %fs
    addl $8, %esp
    iret


# wrapper function (User Level)
halt:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $1, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

execute:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $2, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret
    
read:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    movl $3, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret
    
write:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    movl $4, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret
    
open:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $5, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

close:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $6, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

getargs:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl 12(%esp), %ecx
    movl $7, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

vidmap:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $8, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

set_handler:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl 12(%esp), %ecx
    movl $9, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

sigreturn:
    pushl %ebx      # callee saved
    movl $10, %eax      # move argument to the registers
    int $0x80       # access to the kernel
    popl %ebx
    ret

sigsend:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl 12(%esp), %ecx
    movl $11, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

malloc:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $12, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

free:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $13, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

thread_create:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $14, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret

thread_kill:
    pushl %ebx      # callee saved
    movl 8(%esp), %ebx      # move argument to the registers
    movl $15, %eax
    int $0x80       # access to the kernel
    popl %ebx
    ret
