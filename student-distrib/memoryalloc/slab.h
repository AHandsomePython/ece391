#ifndef SLAB_H
#define SLAB_H

#include "../lib.h"
#include "buddy.h"


#define MAX_SLAB_NUM 15

#define MAX_PAGE_OF_SLAB 4

typedef struct slab_page{
    struct slab_page* next;
    struct slab_page* prev;
    uint32_t item_num;
    uint32_t* free_list;
}slab_page_t;


typedef struct slab{
    uint32_t page_num;  // total number of pages
    uint32_t item_num;     // current number of items
    uint32_t item_size;     // size of an item (only 2^k, min 4B), can not change after create
    uint32_t start_index_off;   // can not change after create
    slab_page_t* slab_pages;   
}slab_t;


slab_t* alloc_free_slab();
int32_t free_slab(slab_t* slab);
slab_t* slab_create(uint32_t objsize);
int32_t slab_exit(slab_t* slab);
int32_t slab_page_init(slab_page_t* slab_page, uint32_t item_size, uint32_t start_index_off);
void* slab_page_obj_alloc(slab_page_t* slab_page, uint32_t item_size);
void* slab_alloc(slab_t* slab);
int32_t slab_free(slab_t* slab, void* ptr);
int32_t slab_page_obj_free(slab_page_t* slab_page, uint32_t item_size, void* ptr);
inline int32_t is_ptr_in_slab_page(slab_page_t* slab_page, void* ptr);
inline int32_t is_slab_page_empty(slab_page_t* slab_page);
////void test_slab();

#endif
