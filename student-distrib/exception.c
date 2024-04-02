#include "exception.h"
#include "lib.h"
#include "syscall.h"
#include "interrupt.h"
#include "text_terminal.h"
#include "signal.h"
#include "process.h"

#define NR_EXCP 32

static void (*exception[NR_EXCP])(void) = {
    sys_divide_by_zero_exception,
    sys_debug_exception,
    sys_non_maskable_interrupt_exception,
    sys_breakpoint_exception,
    sys_overflow_exception,
    sys_bound_range_exceeded_exception,
    sys_bound_range_exceeded_exception,
    sys_device_not_available_exception,
    sys_double_fault_exception,
    sys_coprocessor_segment_overrun_exception,
    sys_invalid_tss_exception,
    sys_segment_not_present_exception,
    sys_stack_segment_fault_exception,
    sys_general_protection_fault_exception,
    sys_page_fault_exception,
    sys_reserved1_exception,
    sys_floating_point_x87_exception,
    sys_alignment_check_exception,
    sys_machine_check_exception,
    sys_simd_floating_point_exception,
    sys_vituralization_exception,
    sys_control_protection_exception,
    sys_reserved2_exception,
    sys_reserved3_exception,
    sys_reserved4_exception,
    sys_reserved5_exception,
    sys_reserved6_exception,
    sys_reserved7_exception,
    sys_hypervisor_injection_exception,
    sys_vmm_communication_exception,
    sys_security_exception,
    sys_reserved8_exception
};

unsigned int do_exception(struct pt_regs* regs){
    if(regs==NULL) return 0;
    int excp_index = regs->orig_eax;
    if(excp_index<0 || excp_index>=NR_EXCP) return 0;
    exception[excp_index]();
    signal_handler(current->signals, regs);
    return 1;
}



void sys_divide_by_zero_exception(){
    kprintf("divide_by_zero_exceptio\n");
    kernal_send_signal(DIV_ZERO, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_debug_exception(){
    kprintf("debug_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_non_maskable_interrupt_exception(){
    kprintf("non_maskable_interrupt_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_breakpoint_exception(){
    kprintf("breakpoint_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_overflow_exception(){
    kprintf("overflow_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_bound_range_exceeded_exception(){
    kprintf("bound_range_exceeded_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_invalid_opcode_exception(){
    kprintf("invalid_opcode_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_device_not_available_exception(){
    kprintf("device_not_available_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_double_fault_exception(){
    kprintf("double_fault_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_coprocessor_segment_overrun_exception(){
    kprintf("coprocessor_segment_overrun_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_invalid_tss_exception(){
    kprintf("invalid_tss_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_segment_not_present_exception(){
    kprintf("segment_not_present_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_stack_segment_fault_exception(){
    kprintf("stack_segment_fault_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_general_protection_fault_exception(){
    kprintf("general_protection_fault_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_page_fault_exception(){
    kprintf("page_fault_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved1_exception(){
    kprintf("reserved1_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_floating_point_x87_exception(){
    kprintf("x87_floating_point_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_alignment_check_exception(){
    kprintf("alignment_check_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_machine_check_exception(){
    kprintf("machine_check_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_simd_floating_point_exception(){
    kprintf("simd_floating_point_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_vituralization_exception(){
    kprintf("vituralization_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_control_protection_exception(){
    kprintf("control_protection_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved2_exception(){
    kprintf("reserved2_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved3_exception(){
    kprintf("reserved3_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved4_exception(){
    kprintf("reserved4_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved5_exception(){
    kprintf("reserved5_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved6_exception(){
    kprintf("reserved6_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved7_exception(){
    kprintf("reserved7_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_hypervisor_injection_exception(){
    kprintf("hypervisor_injection_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_vmm_communication_exception(){
    kprintf("vmm_communication_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_security_exception(){
    kprintf("security_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

void sys_reserved8_exception(){
    kprintf("reserved8_exception\n");
    kernal_send_signal(SEGFAULT, current->pid);
    // halt(EXCEPTION_HALT_STATUS);
}

// // Interrupt handler functions
// void pit_interrupt(){
//     kprintf("pit_interrupt\n");
// }
