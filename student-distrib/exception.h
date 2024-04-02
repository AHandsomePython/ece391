#ifndef EXCEPTION_H
#define EXCEPTION_H
#include "interrupt.h"

#define EXCEPTION_HALT_STATUS 0x99

void sys_divide_by_zero_exception();
void sys_debug_exception();
void sys_non_maskable_interrupt_exception();
void sys_breakpoint_exception();
void sys_overflow_exception();
void sys_bound_range_exceeded_exception();
void sys_invalid_opcode_exception();
void sys_device_not_available_exception();
void sys_double_fault_exception();
void sys_coprocessor_segment_overrun_exception();
void sys_invalid_tss_exception();
void sys_segment_not_present_exception();
void sys_stack_segment_fault_exception();
void sys_general_protection_fault_exception();
void sys_page_fault_exception();
void sys_reserved1_exception();
void sys_floating_point_x87_exception();
void sys_alignment_check_exception();
void sys_machine_check_exception();
void sys_simd_floating_point_exception();
void sys_vituralization_exception();
void sys_control_protection_exception();
void sys_reserved2_exception();
void sys_reserved3_exception();
void sys_reserved4_exception();
void sys_reserved5_exception();
void sys_reserved6_exception();
void sys_reserved7_exception();
void sys_hypervisor_injection_exception();
void sys_vmm_communication_exception();
void sys_security_exception();
void sys_reserved8_exception();


unsigned int do_exception(struct pt_regs* regs);

// Interrupt handler functions
// void pit_interrupt();
// extern void rtc_interrupt();

// System call handler function
// void system_call();
#endif
