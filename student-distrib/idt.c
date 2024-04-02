#include "idt.h"
#include "i8259.h"

// Array stores the linkage functions of interrupts
static void (*interrupt[NR_IRQS])(void) = {interrupt0, interrupt1, interrupt2, interrupt3, interrupt4, interrupt5, interrupt6, interrupt7, interrupt8, interrupt9, interrupt10, interrupt11, interrupt12, interrupt13, interrupt14, interrupt15};

/* 
 *   set_intr_gate
 *   DESCRIPTION: use to set the interrupt gate in IDT 
 *   INPUTS: n - the index of IDT entry
 *           addr - the address of handler function
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array
 */
void set_intr_gate(int n, void* addr){
    // Call SET_IDT_ENTRY macro to init offset
    SET_IDT_ENTRY(idt[n], addr);
    // Use kernal segment
    idt[n].seg_selector = KERNEL_CS;
    // bits 32-39 set to 0 as defult
    idt[n].reserved4 = 0;
    // Set Gate Type as 1110 (32bit trap gate)
    idt[n].reserved3 = 0;
    idt[n].reserved2 = 1;
    idt[n].reserved1 = 1;
    idt[n].size = 1;
    // Set bit 44 as defult 0
    idt[n].reserved0 = 0;
    // Set DPL = 0 (Kernal Privilege Level)
    idt[n].dpl = 0;
    // Present always be 1
    idt[n].present = 1;
}

/* 
 *   set_system_gate
 *   DESCRIPTION: use to set the system gate in IDT 
 *   INPUTS: n - the index of IDT entry
 *           addr - the address of handler function
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array
 */
void set_system_gate(int n, void* addr){
    // Call SET_IDT_ENTRY macro to init offset
    SET_IDT_ENTRY(idt[n], addr);
    // Use kernal segment
    idt[n].seg_selector = KERNEL_CS;
    // bits 32-39 set to 0 as defult
    idt[n].reserved4 = 0;
    // Set Gate Type as 1111 (32bit trap gate)
    idt[n].reserved3 = 1;
    idt[n].reserved2 = 1;
    idt[n].reserved1 = 1;
    idt[n].size = 1;
    // Set bit 44 as defult 0
    idt[n].reserved0 = 0;
    // Set DPL = 3 (User Privilege Level)
    idt[n].dpl = 3;
    // Present always be 1
    idt[n].present = 1;
}

/* 
 *   set_system_intr_gate
 *   DESCRIPTION: use to set the system interrupt gate in IDT 
 *   INPUTS: n - the index of IDT entry
 *           addr - the address of handler function
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array
 */
void set_system_intr_gate(int n, void* addr){
    // Call SET_IDT_ENTRY macro to init offset
    SET_IDT_ENTRY(idt[n], addr);
    // Use kernal segment
    idt[n].seg_selector = KERNEL_CS;
    // bits 32-39 set to 0 as defult
    idt[n].reserved4 = 0;
    // Set Gate Type as 1110 (32 bits intr gate)
    idt[n].reserved3 = 0;
    idt[n].reserved2 = 1;
    idt[n].reserved1 = 1;
    idt[n].size = 1;
    // Set bit 44 as defult 0
    idt[n].reserved0 = 0;
    // Set DPL = 3 (User Privilege Level)
    idt[n].dpl = 3;
    // Present always be 1
    idt[n].present = 1;
}

/* 
 *   set_trap_gate
 *   DESCRIPTION: use to set the trap gate in IDT 
 *   INPUTS: n - the index of IDT entry
 *           addr - the address of handler function
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array
 */
void set_trap_gate(int n, void* addr){
    // Call SET_IDT_ENTRY macro to init offset
    SET_IDT_ENTRY(idt[n], addr);
    // Use kernal segment
    idt[n].seg_selector = KERNEL_CS;
    // bits 32-39 set to 0 as defult
    idt[n].reserved4 = 0;
    // Set Gate Type as 1111 (32 bits trap gate)
    idt[n].reserved3 = 1;
    idt[n].reserved2 = 1;
    idt[n].reserved1 = 1;
    idt[n].size = 1;
    // Set bit 44 as defult 0
    idt[n].reserved0 = 0;
    // Set DPL = 0 (Kernal Privilege Level)
    idt[n].dpl = 0;
    // Present always be 1
    idt[n].present = 1;
}

/* 
 *   set_task_gate
 *   DESCRIPTION: use to set the task gate in IDT 
 *   INPUTS: n - the index of IDT entry
 *           addr - the address of handler function
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array
 */
void set_task_gate(int n, void* addr){
    // Call SET_IDT_ENTRY macro to init offset
    SET_IDT_ENTRY(idt[n], addr);
    // Use kernal segment
    idt[n].seg_selector = KERNEL_CS;
    // bits 32-39 set to 0 as defult
    idt[n].reserved4 = 0;
    // Set Gate Type as 0101 (16 bits task gate)
    idt[n].reserved3 = 1;
    idt[n].reserved2 = 0;
    idt[n].reserved1 = 1;
    idt[n].size = 0;
    // Set bit 44 as defult 0
    idt[n].reserved0 = 0;
    // Set DPL = 0 (Kernal Privilege Level)
    idt[n].dpl = 0;
    // Present always be 1
    idt[n].present = 1;
}

/* 
 *   init_idt
 *   DESCRIPTION: use to init the IDT, load the entries in IDT 
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the idt array, and set the IDTR register
 */
void init_idt(){
    // Init Exceptions in IDT (number 0x0-0x1F is the exception vectors)
    set_intr_gate(0x0, divide_by_zero_exception);
    set_intr_gate(0x1, debug_exception);
    set_intr_gate(0x2, non_maskable_interrupt_exception);
    set_system_intr_gate(0x3, breakpoint_exception);
    set_system_gate(0x4, overflow_exception);
    set_system_gate(0x5, bound_range_exceeded_exception);
    set_intr_gate(0x6, invalid_opcode_exception);
    set_intr_gate(0x7, device_not_available_exception);
    set_intr_gate(0x8, double_fault_exception);
    set_intr_gate(0x9, coprocessor_segment_overrun_exception);
    set_intr_gate(0xA, invalid_tss_exception);
    set_intr_gate(0xB, segment_not_present_exception);
    set_intr_gate(0xC, stack_segment_fault_exception);
    set_intr_gate(0xD, general_protection_fault_exception);
    set_intr_gate(0xE, page_fault_exception);
    set_intr_gate(0xF, reserved1_exception);
    set_intr_gate(0x10, floating_point_x87_exception);
    set_intr_gate(0x11, alignment_check_exception);
    set_intr_gate(0x12, machine_check_exception);
    set_intr_gate(0x13, simd_floating_point_exception);
    set_intr_gate(0x14, vituralization_exception);
    set_intr_gate(0x15, control_protection_exception);
    set_intr_gate(0x16, reserved2_exception);
    set_intr_gate(0x17, reserved3_exception);
    set_intr_gate(0x18, reserved4_exception);
    set_intr_gate(0x19, reserved5_exception);
    set_intr_gate(0x1A, reserved6_exception);
    set_intr_gate(0x1B, reserved7_exception);
    set_intr_gate(0x1C, hypervisor_injection_exception);
    set_intr_gate(0x1D, vmm_communication_exception);
    set_intr_gate(0x1E, security_exception);
    set_intr_gate(0x1F, reserved8_exception);
    // Init Interrupts in IDT (the first input is interrupt vector, the second input interrupt[vector# - base#] is the corrent linkage function)
    // set_intr_gate(PIT_INT_VEC, interrupt[PIT_INT_VEC-INTERRUPT_BASE_VEC]);
    // set_intr_gate(KEYBOARD_INT_VEC, interrupt[KEYBOARD_INT_VEC-INTERRUPT_BASE_VEC]);
    // set_intr_gate(RTC_INT_VEC, interrupt[RTC_INT_VEC-INTERRUPT_BASE_VEC]);
    int i;
    for(i=0;i<NR_IRQS;i++){
        set_intr_gate(0x20 + i, interrupt[i]);  // 0x20 is the start vector for interrupt
    }
    // Init System Call in IDT (vec 0x80 is system call)
    set_system_gate(0x80, system_call_link);
    // Load IDT
    lidt(idt_desc_ptr);
}
