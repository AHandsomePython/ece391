#define ASM     1

.text

.global	do_exception


.global divide_by_zero_exception
.global debug_exception
.global non_maskable_interrupt_exception
.global breakpoint_exception
.global overflow_exception
.global bound_range_exceeded_exception
.global invalid_opcode_exception
.global device_not_available_exception
.global double_fault_exception
.global coprocessor_segment_overrun_exception
.global invalid_tss_exception
.global segment_not_present_exception
.global stack_segment_fault_exception
.global general_protection_fault_exception
.global page_fault_exception
.global reserved1_exception
.global floating_point_x87_exception
.global alignment_check_exception
.global machine_check_exception
.global simd_floating_point_exception
.global vituralization_exception
.global control_protection_exception
.global reserved2_exception
.global reserved3_exception
.global reserved4_exception
.global reserved5_exception
.global reserved6_exception
.global reserved7_exception
.global hypervisor_injection_exception
.global vmm_communication_exception
.global security_exception
.global reserved8_exception

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



divide_by_zero_exception:
    pushl $0
    pushl $0
    jmp common_exception

debug_exception:
    pushl $0
    pushl $1
    jmp common_exception

non_maskable_interrupt_exception:
    pushl $0
    pushl $2
    jmp common_exception

breakpoint_exception:
    pushl $0
    pushl $3
    jmp common_exception

overflow_exception:
    pushl $0
    pushl $4
    jmp common_exception

bound_range_exceeded_exception:
    pushl $0
    pushl $5
    jmp common_exception

invalid_opcode_exception:
    pushl $0
    pushl $6
    jmp common_exception

device_not_available_exception:
    pushl $0
    pushl $7
    jmp common_exception

double_fault_exception:
    pushl $8
    jmp common_exception

coprocessor_segment_overrun_exception:
    pushl $0
    pushl $9
    jmp common_exception

invalid_tss_exception:
    pushl $10
    jmp common_exception

segment_not_present_exception:
    pushl $11
    jmp common_exception

stack_segment_fault_exception:
    pushl $12
    jmp common_exception

general_protection_fault_exception:
    pushl $13
    jmp common_exception

page_fault_exception:
    pushl $14
    jmp common_exception

reserved1_exception:
    pushl $0
    pushl $15
    jmp common_exception

floating_point_x87_exception:
    pushl $0
    pushl $16
    jmp common_exception

alignment_check_exception:
    pushl $17
    jmp common_exception

machine_check_exception:
    pushl $0
    pushl $18
    jmp common_exception

simd_floating_point_exception:
    pushl $0
    pushl $19
    jmp common_exception

vituralization_exception:
    pushl $0
    pushl $20
    jmp common_exception

control_protection_exception:
    pushl $0
    pushl $21
    jmp common_exception

reserved2_exception:
    pushl $0
    pushl $22
    jmp common_exception

reserved3_exception:
    pushl $0
    pushl $23
    jmp common_exception

reserved4_exception:
    pushl $0
    pushl $24
    jmp common_exception

reserved5_exception:
    pushl $0
    pushl $25
    jmp common_exception

reserved6_exception:
    pushl $0
    pushl $26
    jmp common_exception

reserved7_exception:
    pushl $0
    pushl $27
    jmp common_exception

hypervisor_injection_exception:
    pushl $0
    pushl $28
    jmp common_exception

vmm_communication_exception:
    pushl $0
    pushl $29
    jmp common_exception

security_exception:
    pushl $0
    pushl $30
    jmp common_exception

reserved8_exception:
    pushl $0
    pushl $31
    jmp common_exception


common_exception:
    SAVE_ALL    # Save all regs
    movl %esp, %eax     # move %esp to %eax, now %eax store the pointers point to the pt_regs struct
    pushl %eax      # pass the arg to do_exception
    call do_exception     # call do_exception
    addl $4, %esp   # pop the arg
    jmp ret_from_exception   # jmp to ret_from_exception

ret_from_exception:
    RESTORE_ALL
    addl $8, %esp
    IRET
