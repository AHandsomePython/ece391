#include "malloc.h"
#include "slab.h"
#include "../lib.h"
#include "buddy.h"

#define MALLOC_SLAB_NUM 11

static slab_t* malloc_slab[MALLOC_SLAB_NUM];

int32_t malloc_driver_init(){
    malloc_slab[0] = slab_create(4);    // 1B
    malloc_slab[1] = malloc_slab[0];    // 2B
    malloc_slab[2] = malloc_slab[0];    // 4B
    int i;
    for(i=3; i<MALLOC_SLAB_NUM; i++){
        malloc_slab[i] = slab_create(1<<i);
    }
    buddy_init(&buddy_sys, MALLOC_ADDR);
    return 0;
}

uint32_t get_power_of_two(uint32_t val){
    val--;
    val |= (val >> 1);
    val |= (val >> 2);
    val |= (val >> 4);
    val |= (val >> 8);
    val |= (val >> 16);
    return val + 1;
}

void* kmalloc(uint32_t size){
    if(size>1024) return NULL;
    uint32_t size_power2 = get_power_of_two(size);
    int order = 0;
    while ((1<<order) < size_power2)    // notice
    {
        order++;
    }
    if (order > 10){
        //kprintf("too large space error\n");
        return NULL;
    }
    void* result = slab_alloc(malloc_slab[order]);
    return result;
}

int32_t kfree(void* ptr){
    int i, res;
    if (ptr == NULL){
        //kprintf("cannot free NULL pointer");
        return -1;
    }
    for (i=0;i<MALLOC_SLAB_NUM;i++){
        slab_page_t* slab_temp = malloc_slab[i]->slab_pages;
        if(slab_temp==NULL) continue;
        if(slab_temp->next==slab_temp){
            if(is_ptr_in_slab_page(slab_temp, ptr)){
                res = slab_free(malloc_slab[i], ptr);
                return res;
            }
            continue;
        }
        for (slab_temp = malloc_slab[i]->slab_pages; slab_temp->next!=malloc_slab[i]->slab_pages; slab_temp = slab_temp->next){
            if (is_ptr_in_slab_page(slab_temp, ptr)){
                res = slab_free(malloc_slab[i], ptr);
                return res;
            }
        }
    }
    return -1;
}

