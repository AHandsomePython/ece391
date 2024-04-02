#ifndef IDT_H
#define IDT_H

#include "x86_desc.h"
#include "exception.h"
// Interrupt Vector
#define INTERRUPT_BASE_VEC 0x20     // The small interrupt vector of interrupt
#define PIT_INT_VEC 0x20            // interrupt vector of PIT
#define KEYBOARD_INT_VEC 0x21       // interrupt vector of keyboard
#define RTC_INT_VEC 0x28            // interrupt vector of RTC

void set_intr_gate(int n, void* addr);  // func used to set intrrupt gate
void set_system_gate(int n, void* addr);    // func used to set system gate
void set_system_intr_gate(int n, void* addr);   // func used to set system intrrupt gate
void set_trap_gate(int n, void* addr);      // func used to set trap gate
void set_task_gate(int n, void* addr);      // func used to set task gate

void init_idt();    // idt init function

// Exception handler functions (defined outside)
extern void divide_by_zero_exception();
extern void debug_exception();
extern void non_maskable_interrupt_exception();
extern void breakpoint_exception();
extern void overflow_exception();
extern void bound_range_exceeded_exception();
extern void invalid_opcode_exception();
extern void device_not_available_exception();
extern void double_fault_exception();
extern void coprocessor_segment_overrun_exception();
extern void invalid_tss_exception();
extern void segment_not_present_exception();
extern void stack_segment_fault_exception();
extern void general_protection_fault_exception();
extern void page_fault_exception();
extern void reserved1_exception();
extern void floating_point_x87_exception();
extern void alignment_check_exception();
extern void machine_check_exception();
extern void simd_floating_point_exception();
extern void vituralization_exception();
extern void control_protection_exception();
extern void reserved2_exception();
extern void reserved3_exception();
extern void reserved4_exception();
extern void reserved5_exception();
extern void reserved6_exception();
extern void reserved7_exception();
extern void hypervisor_injection_exception();
extern void vmm_communication_exception();
extern void security_exception();
extern void reserved8_exception();

// System call handler function
extern void system_call_link(void);

// Interrupt linkage functions (defined in interrupt_link.S)
extern void interrupt0(void);
extern void interrupt1(void);
extern void interrupt2(void);
extern void interrupt3(void);
extern void interrupt4(void);
extern void interrupt5(void);
extern void interrupt6(void);
extern void interrupt7(void);
extern void interrupt8(void);
extern void interrupt9(void);
extern void interrupt10(void);
extern void interrupt11(void);
extern void interrupt12(void);
extern void interrupt13(void);
extern void interrupt14(void);
extern void interrupt15(void);

#endif
