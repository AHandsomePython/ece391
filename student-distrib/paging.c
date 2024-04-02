#include "paging.h"
#include "process.h"

inline void init_paging_ctr_regs();     // function used to init the control registers
inline void reload_tlb();   // function used to reload the TLB

/* 
 *   init_paging
 *   DESCRIPTION: used to init and open paging
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change page_directory and page_table0 array, will change the control registers, and will enable paging
 */
void init_paging(){
    uint32_t i;
    //initialize the page directory
    for(i=0;i<PAGE_DIR_ENTRY_NUM;i++){
        page_directory[i].val = 0;  // clear entry
        page_directory[i].rw = 1;   // set read and write flag
    }
    //initialize the page table
    for(i=0;i<PAGE_TAB_ENTRY_NUM;i++){
        page_table0[i].val = 0;     // clear entrt
        page_table0[i].rw = 1;      // set read and write flag
        page_table0[i].base_addr = i;   // set the base address as the entry index (4kB aligned)
    
        page_table_vga[i].val = 0;
        page_table_vga[i].rw = 1;

        page_table_modex[i].val = 0;
        page_table_modex[i].rw = 1;
    }
    // Set the kernal page (4MB), set present present, read and flag, 4MB size, global flags, and set the aligned address
    page_directory[get_dir_entry(KERNAL_ADDR)].val = (PAGEF_PRESENT | PAGEF_RW | PAGEF_SIZE | PAGEF_GLOBAL | (KERNAL_ADDR & ALIGNED_ADDR_MASK));
    // Set the VGA page, first set the page directory entry, set the present, rw, global flags, and set the aligned address as the page_table0's address
    // Then, set the page_table0. Set the present, rw flags, and set the aligned address as the vga text buffer's address
    // page_directory[get_dir_entry(VGA_TEXT_BUF_ADDR)].val = (PAGEF_PRESENT | PAGEF_RW | PAGEF_GLOBAL | ((uint32_t)page_table0 & ALIGNED_ADDR_MASK));      // First page
    // page_table0[get_pag_entry(VGA_TEXT_BUF_ADDR)].val = (PAGEF_PRESENT | PAGEF_RW | (VGA_TEXT_BUF_ADDR & ALIGNED_ADDR_MASK));
    // 80 rows 25 columns 2 chars each character, 160 is left for more space 
    for (i = VGA_TEXT_BUF_ADDR; i<= (VGA_TEXT_BUF_ADDR2+80*25*2+160); i+=0x1000){
        page_directory[get_dir_entry(i)].val = (PAGEF_PRESENT | PAGEF_RW | PAGEF_GLOBAL | ((uint32_t)page_table0 & ALIGNED_ADDR_MASK));
        page_table0[get_pag_entry(i)].val = (PAGEF_PRESENT | PAGEF_RW | (i & ALIGNED_ADDR_MASK));
    }
    // Set Modex Paging
    for(i=VGA_MODEX_ADDR; i<0xB0000; i+=0x1000){
        page_table0[get_pag_entry(i)].val = (PAGEF_PRESENT | PAGEF_RW | (i & ALIGNED_ADDR_MASK));
    }
    // Init paging by seting the control registers
    init_paging_ctr_regs();
}

/* 
 *   init_paging_ctr_regs
 *   DESCRIPTION: initialize the page control registers
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the control registers, and will enable paging
 */
inline void init_paging_ctr_regs(){   // Referance : https://wiki.osdev.org/Paging - Enabling
    asm volatile(
        "movl $page_directory, %%eax \n\t"          // move page_directory's address to cr3(PBDR)
        "movl %%eax, %%cr3 \n\t"
        "movl %%cr4, %%eax \n\t"
        "orl  $0x10, %%eax \n\t"                    // set bit 4 in cr4 to 1 to enable mixed size pages
        "movl %%eax, %%cr4 \n\t"
        "movl %%cr0, %%eax \n\t"
        "orl  $0x80000001, %%eax \n\t"              // set the bit 0 in cr0 and highest bit in cr0 to enable paging and set protection
        "movl %%eax, %%cr0 \n\t"
        :   // No output regs
        :   // No vars
        : "%eax"    // %eax changed
        );
}

/* 
 *   reload_tlb
 *   DESCRIPTION: used to reload TLB
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will reload the cr3
 */
inline void reload_tlb(){
    asm volatile(
        "movl %%cr3, %%eax \n\t"        // reload cr3 to reload TLB
        "movl %%eax, %%cr3 \n\t"
        :   // No output regs
        :   // No vars
        : "%eax"    // %eax changed
    );
}


int32_t map_4MB_page(uint32_t virtual_addr, uint32_t physical_addr){
    if(virtual_addr==0 || physical_addr==0) return -1;
    uint32_t pd_index = get_dir_entry(virtual_addr);
    // if(page_directory[pd_index].present) return -1;
    page_directory[pd_index].val = 0;
    page_directory[pd_index].present = 1;
    page_directory[pd_index].rw = 1;
    page_directory[pd_index].user_supervisor = 1;
    page_directory[pd_index].size = 1;
    page_directory[pd_index].val |= physical_addr;
    reload_tlb();
    return 0;
}

int32_t map_vid_page(int pid){
    if(pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    PCB_t* pcb = get_pcb_from_pos(pid);
    terminal_t* terminal = pcb->terminal;
    int index = terminal_get_index(terminal);
    if (index<0 || index >2 ){
        return -1;
    }
    uint32_t addr = get_addr(index);
    
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val = 0;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].present = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].rw = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].user_supervisor = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val |= ((uint32_t)page_table_vga);
    // Set page table
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val = 0;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].present = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].rw = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].user_supervisor = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val |= ((uint32_t)addr);
    // reload tlb
    reload_tlb();
    // screen_printf(&screen,"sys_vidmap\n");
    return 0;
}
