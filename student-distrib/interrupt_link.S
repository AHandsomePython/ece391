.text

# Set the gloable variables
.globl do_IRQ, pit_interrupt, keyboard_interrupt, rtc_interrupt
.globl interrupt0, interrupt1, interrupt2, interrupt3, interrupt4, interrupt5, interrupt6, interrupt7, interrupt8, interrupt9, interrupt10, interrupt11, interrupt12, interrupt13, interrupt14, interrupt15

# a macro SAVE_ALL to sava all regs on the stack to create the pt_regs struct
.macro SAVE_ALL
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
.endm

# a macro RESTORE_ALL to restore all regs to restore the regs back
.macro RESTORE_ALL
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    popl %ds
    popl %es
    popl %fs
.endm

# IRQ jmp functions:
# interrupt[#irq] set as the entrance address of interrupt gates intr gate irq+20
# when interrupt happens, it will push the ~irq to the stack and jmp to common_interrupt
interrupt0:
    pushl $0
    pushl $~0
    jmp common_interrupt

interrupt1:
    pushl $0
    pushl $~1
    jmp common_interrupt

interrupt2:
    pushl $0
    pushl $~2
    jmp common_interrupt

interrupt3:
    pushl $0
    pushl $~3
    jmp common_interrupt

interrupt4:
    pushl $0
    pushl $~4
    jmp common_interrupt

interrupt5:
    pushl $0
    pushl $~5
    jmp common_interrupt

interrupt6:
    pushl $0
    pushl $~6
    jmp common_interrupt

interrupt7:
    pushl $0
    pushl $~7
    jmp common_interrupt

interrupt8:
    pushl $0
    pushl $~8
    jmp common_interrupt

interrupt9:
    pushl $0
    pushl $~9
    jmp common_interrupt

interrupt10:
    pushl $0
    pushl $~10
    jmp common_interrupt

interrupt11:
    pushl $0
    pushl $~11
    jmp common_interrupt

interrupt12:
    pushl $0
    pushl $~12
    jmp common_interrupt

interrupt13:
    pushl $0
    pushl $~13
    jmp common_interrupt

interrupt14:
    pushl $0
    pushl $~14
    jmp common_interrupt

interrupt15:
    pushl $0
    pushl $~15
    jmp common_interrupt

# common_interrupt function
common_interrupt:
    SAVE_ALL    # Save all regs
    movl %esp, %eax     # move %esp to %eax, now %eax store the pointers point to the pt_regs struct
    pushl %eax      # pass the arg to do_IRQ
    call do_IRQ     # call do_IRQ
    addl $4, %esp   # pop the arg
    jmp ret_from_intr   # jmp to ret_from_intr

ret_from_intr:
    RESTORE_ALL     # restore all regs
    addl $8, %esp   # pop the ~irq & dummy
    # addl $4, %esp   # pop the ~irq
    IRET            # return from the interrupt
