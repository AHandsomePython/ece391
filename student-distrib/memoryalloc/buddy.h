#ifndef BUDDY_H
#define BUDDY_H

#include "../lib.h"
#include "../process.h"

#define BUDDY_FREE_BITMAP_SIZE 32
#define BUDDY_PAIR_BITMAP_SIZE 36
#define PAGE_NUMS 1024

#define MALLOC_ADDR (0x800000+0x400000*(MAX_PROCESS_NUM + 1))

// Buddy System
typedef struct buddy{
    uint32_t page_off;
    uint32_t free_bitmap[BUDDY_FREE_BITMAP_SIZE];
    uint32_t pair_bitmap[BUDDY_PAIR_BITMAP_SIZE];    // (2^10-1)/32
} buddy_t;
int pair_to_free(int order, int n, int flag);
int pair_get_child_off(int order, int n, int flag);
int pair_get_parent_off(int order, int n);
int pair_bit_map_free_test(buddy_t* buddy, int order, int n);
int32_t buddy_init(buddy_t* buddy, uint32_t page_off);
int32_t buddy_alloc(buddy_t* buddy, int order);
void page_free_set(buddy_t* buddy, int order, int n, int flag);
int32_t page_free_test(buddy_t* buddy, int order, int n, int flag);
int32_t block_available(buddy_t* buddy, int order);
int32_t buddy_free(buddy_t* buddy, uint32_t free_map_index, int order);

void* page_alloc(int order);
void page_free(void* ptr, int order);
uint32_t free_map_index_get(void* ptr, buddy_t* buddy);

int malloc_init();


typedef struct ptr_tabel
{
    void* ptr;
    int order;
    int free_map_index;
} ptr_tabel_t;


ptr_tabel_t ptr_map[PAGE_NUMS];
buddy_t buddy_sys;
#endif
