#ifndef PAGING_H
#define PAGING_H

#include "lib.h"
#include "text_terminal.h"

// Paging flag define (bit masks)
#define PAGEF_PRESENT            0x1
#define PAGEF_RW                 0x2
#define PAGEF_USER_SUPERVISOR    0x4
#define PAGEF_PWT                0x8
#define PAGEF_PCD                0x10
#define PAGEF_ACCESSED           0x20
#define PAGEF_DIRTY              0x40
#define PAGEF_SIZE               0x80
#define PAGEF_GLOBAL             0x100
#define PAGEF_AVAILABLE          0xE00

// Number of entriy in page directory and page table
#define PAGE_DIR_ENTRY_NUM       1024
#define PAGE_TAB_ENTRY_NUM       1024

// Macro for 4KB and 4MB
#define PAGE_4KB_OFF             12                         // The bit start of 4KB
#define PAGE_4MB_OFF             22                         // The bit start of 4MB
#define PAGE_4KB_VAL             (1<<PAGE_4KB_OFF)          // The value of 4MB
#define PAGE_4MB_VAL             (1<<PAGE_4MB_OFF)          // The value of 4MB
#define PAGE_10BITS_MASK         0x03FF                     // Used to get low 10 bits
#define PAGE_8MB_OFF             23
#define PAGE_8MB_VAL             (1<<PAGE_8MB_OFF)

#define VALUE_128MB              0x8000000                  // 128 mb
#define VALUE_132MB              0x8400000                  // 132 mb

// Mask to get the component from vitural addr
#define VIRTUAL_DIR_MASK         0xFFC00000                 // Used to get the 10 MSBs
#define VIRTUAL_PAG_MASK         0x003FF000                 // Used to get the second 10 MSBs
#define VIRTUAL_OFF_MASK         0x00000FFF                 // Used to get the 12 LSBs
#define ALIGNED_ADDR_MASK        0xFFFFF000                 // Used to get the 20 LSBs
// Offset to get the component from virtural addr
#define VIRTUAL_DIR_OFF          22                         // Bits offset of page directory in virtual address
#define VIRTUAL_PAG_OFF          12                         // Bits offset of page table in virtual address
#define ALIGNED_ADDR_OFF         12                         // Bits offset of aligned address

// Addrs
#define KERNAL_ADDR              PAGE_4MB_VAL               // Kernal start at 4MB in physical memory
//#define VGA_TEXT_BUF_ADDR        (0xB8000)                   // Text buffer of VGA start at 0xB8000, Referance : https://en.wikipedia.org/wiki/VGA_text_mode - Access methods
#define VGA_MODEX_ADDR           0x0A0000

#define PROGRAM_START_VIRTUAL_ADDR 0x08048000

#define PROGRAM_VGA_VIRTUAL_ADDR    0x0D000000
#define PROGRAM_MODEX_VIRTUAL_ADDR    0x0E000000              // 待确定

#define get_dir_entry(vir_addr)      (((uint32_t)vir_addr>>VIRTUAL_DIR_OFF)&PAGE_10BITS_MASK)   // macro used to get the page directory index from virtual address
#define get_pag_entry(vir_addr)      (((uint32_t)vir_addr>>VIRTUAL_PAG_OFF)&PAGE_10BITS_MASK)   // macro used to get the page table index from virtual address

//unit struct in PTE
typedef union PTE{
    uint32_t val;
    struct {
        uint32_t present         :1;    // present flag
        uint32_t rw              :1;    // read and write flag
        uint32_t user_supervisor :1;    // user or supervisor flag
        uint32_t pwt             :1;    // pwt flag
        uint32_t pcd             :1;    // pcd flag
        uint32_t accessed        :1;    // accessed flag
        uint32_t dirty           :1;    // dirty flag
        uint32_t size            :1;    // 4MB or 4KB size flag
        uint32_t global          :1;    // global flag
        uint32_t available       :3;    // available flag
        uint32_t base_addr       :20;   // base address (4KB aligned)
    } __attribute__ ((packed));
} pte_t;

//unit struct in PDE
typedef union PDE{
    uint32_t val;
    struct {
        uint32_t present         :1;    // present flag
        uint32_t rw              :1;    // read and write flag
        uint32_t user_supervisor :1;    // user or supervisor flag
        uint32_t pwt             :1;    // pwt flag
        uint32_t pcd             :1;    // pcd flag
        uint32_t accessed        :1;    // accessed flag
        uint32_t dirty           :1;    // dirty flag
        uint32_t size            :1;    // 4MB or 4KB size flag
        uint32_t global          :1;    // global flag
        uint32_t available       :3;    // available flag
        uint32_t base_addr       :20;   // base address (4KB aligned)
    } __attribute__ ((packed));
} pde_t;

// page_directory
pde_t page_directory[PAGE_DIR_ENTRY_NUM] __attribute__((aligned (PAGE_4KB_VAL)));
// first page table
pte_t page_table0[PAGE_TAB_ENTRY_NUM] __attribute__((aligned (PAGE_4KB_VAL)));
// page table vga
pte_t page_table_vga[PAGE_TAB_ENTRY_NUM] __attribute__((aligned (PAGE_4KB_VAL)));
// page table for modex
pte_t page_table_modex[PAGE_TAB_ENTRY_NUM] __attribute__((aligned (PAGE_4KB_VAL)));
// Paging functions
void init_paging();
inline void reload_tlb();
int32_t map_4MB_page(uint32_t virtual_addr, uint32_t physical_addr);
int32_t map_vid_page(int pid);

#endif

