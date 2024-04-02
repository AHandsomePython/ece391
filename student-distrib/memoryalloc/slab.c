#include "slab.h"
#include "buddy.h"
#include "../paging.h"

static slab_t slab_arr[MAX_SLAB_NUM];
static int32_t slab_in_use[MAX_SLAB_NUM];

#define ptr_next(ptr, size) ((void*)(((uint32_t)ptr) + (uint32_t)size))

slab_t* alloc_free_slab(){
    int i;
    for(i=0; i<MAX_SLAB_NUM; i++){
        if(slab_in_use[i]==0){
            slab_in_use[i] = 1;
            return &slab_arr[i];
        }
    }
    return NULL;
}

int32_t free_slab(slab_t* slab){
    int32_t index = ((uint32_t)slab - (uint32_t)slab_arr) / sizeof(slab_t);
    if(index<0 || index>MAX_SLAB_NUM) return -1;
    slab_in_use[index] = 0;
    return 0;
}

slab_t* slab_create(uint32_t objsize){
    if(objsize == 0) return NULL;
    if(objsize < 4 || objsize>1024) return NULL;
    slab_t* slab = alloc_free_slab();
    if(slab==NULL) return NULL;
    slab->page_num = 0;
    slab->item_num = 0;
    slab->item_size = objsize;
    if(objsize<=4) slab->start_index_off = 4;
    else if(objsize<=8) slab->start_index_off = 2;
    else slab->start_index_off = 1;
    slab->slab_pages = NULL;
    return slab;
}

int32_t slab_exit(slab_t* slab){
    if(slab==NULL) return -1;
    free_slab(slab);
    if(slab->slab_pages==NULL) return 0;
    slab_page_t* page = slab->slab_pages;
    slab_page_t* next = page->next;
    page_free(page, 0);
    while(next!=slab->slab_pages){
        page = next;
        next = page->next;
        page_free(page, 0);
    }
    slab->slab_pages = NULL;
    return 0;
}

// inline int32_t is_slab_page_full(slab_page_t* slab_page, uint32_t item_size, uint32_t start_index_off){
//     return PAGE_4KB_VAL / item_size - start_index_off == slab_page->item_num;
// }

int32_t slab_page_init(slab_page_t* slab_page, uint32_t item_size, uint32_t start_index_off){
    slab_page->item_num = 0;
    slab_page->free_list = (void*)((uint32_t)(slab_page) + (uint32_t)item_size * start_index_off);
    uint32_t* ptr = slab_page->free_list;
    uint32_t max_num = PAGE_4KB_VAL / item_size - start_index_off;
    uint32_t i;
    for(i=0; i<max_num-1; i++){
        *ptr = (uint32_t)ptr_next(ptr, item_size);
        ptr = (uint32_t*)(*ptr);
    }
    *ptr = NULL;
    return 1;
}

void* slab_page_obj_alloc(slab_page_t* slab_page, uint32_t item_size){
    if(slab_page->free_list==NULL) return NULL;
    uint32_t* ptr = slab_page->free_list;
    slab_page->free_list = (uint32_t*)(*ptr);
    slab_page->item_num++;
    memset(ptr, 0, item_size);
    return ptr;
}

void* slab_alloc(slab_t* slab){
    void* ptr = NULL;
    // first alloc
    if(slab->slab_pages==NULL){
        // init first slab page
        slab->slab_pages = page_alloc(0);
        if(slab->slab_pages==NULL) return NULL;
        slab->slab_pages->next = slab->slab_pages;
        slab->slab_pages->prev = slab->slab_pages;
        slab_page_init(slab->slab_pages, slab->item_size, slab->start_index_off);
        // add item
        slab->item_num++;
        ptr = slab_page_obj_alloc(slab->slab_pages, slab->item_size);
        return ptr;
    }
    
    slab_page_t* slab_page = slab->slab_pages;
    ptr = slab_page_obj_alloc(slab->slab_pages, slab->item_size);
    if(ptr!=NULL){
        slab->item_num++;
        return ptr;
    }
    for(slab_page = slab->slab_pages->next; slab_page->next!=slab->slab_pages; slab_page = slab_page->next){
        ptr = slab_page_obj_alloc(slab_page, slab->item_size);
        if(ptr!=NULL){
            slab->item_num++;
            slab->slab_pages = slab_page;
            return ptr;
        }
    }
    // if all the pages are full
    slab_page_t* new_page = page_alloc(0);
    if(new_page==NULL) return NULL;
    new_page->next = slab->slab_pages;
    new_page->prev = slab->slab_pages->prev;
    new_page->next->prev = new_page;
    new_page->prev->next = new_page;
    slab_page_init(new_page, slab->item_size, slab->start_index_off);
    slab->slab_pages = new_page;
    ptr = slab_page_obj_alloc(slab->slab_pages, slab->item_size);
    slab->item_num++;
    return ptr;
}

int32_t slab_page_obj_free(slab_page_t* slab_page, uint32_t item_size, void* ptr){
    *((uint32_t*)ptr) = (uint32_t)slab_page->free_list;
    slab_page->free_list = ptr;
    slab_page->item_num--;
    return 0;
}

inline int32_t is_ptr_in_slab_page(slab_page_t* slab_page, void* ptr){
    return (((uint32_t)ptr & 0xFFFFF000) == (uint32_t)slab_page);
}

inline int32_t is_slab_page_empty(slab_page_t* slab_page){
    return slab_page->item_num == 0;
}

int32_t slab_free(slab_t* slab, void* ptr){
    if(slab==NULL || slab->slab_pages==NULL) return -1;
    slab_page_t* page = NULL;
    if(is_ptr_in_slab_page(slab->slab_pages, ptr)){
        slab_page_obj_free(slab->slab_pages, slab->item_size, ptr);
        slab->item_num--;
        if(is_slab_page_empty(slab->slab_pages)){
            if(slab->slab_pages->next == slab->slab_pages){
                page_free(slab->slab_pages, 0);
                slab->slab_pages = NULL;
                return 0;
            }else{
                page = slab->slab_pages;
                slab->slab_pages = page->next;
                page->next->prev = page->prev;
                page->prev->next = page->next;
                page_free(page, 0);
                return 0;
            }
        }
        return 0;
    }

    for(page = slab->slab_pages->next; page!=slab->slab_pages; page = page->next){
        if(is_ptr_in_slab_page(page, ptr)){
            slab_page_obj_free(page, slab->item_size, ptr);
            slab->item_num--;
            if(is_slab_page_empty(page)){
                page->next->prev = page->prev;
                page->prev->next = page->next;
                page_free(page, 0);
                return 0;
            }
            return 0;
        }
    }
    return -1;
}

// void test_slab(){
//     // slab_alloc(&slab_arr[2]);
//     // int*k = (int*)slab_alloc(&slab_arr[2]);
//     // int*b = (int*)slab_alloc(&slab_arr[2]);
//     // *k = 3;
//     // *b = 4;
//     // printf("a is %d, pointer a is %d,  b is %d, pointer b is %d\n", *a, a, *b, b);
//     // slab_free(&slab_arr[2], (void*)a);
//     // slab_free(&slab_arr[2], (void*)b);
//     // int*c = (int*)slab_alloc(&slab_arr[2]);
//     // *c = 6;
//     // printf("c is %d , pointer c is %d\n", *c, c);
//     // slab_free(&slab_arr[2], (void*)c);

//     int num = 2000;
//     int** a[num];
//     int i;
//     slab_t* slab = slab_create(4);
//     for (i=0;i<num;i++){
//         a[i] = (int*)slab_alloc(slab);
//         *((int*)a[i]) = i;
//         //printf(" pointer is %d\n", a[i] );
//         //printf("inside is %d\n", *((int*)a[i]));
//     }


//     // for (i = 1015;i<1025;i++){
//     //     printf("i is %d, pointer is %d\n",i,  a[i]);
//     // }
//     int j;
//     for (i=0;i<num-1;i++){
//         slab_free(slab, a[i]);
//         printf("items are %d          \n", slab->item_num);
        
//     }
//     slab_free(slab, a[num-1]);
//     slab->slab_pages->free_list;
//     //printf("free list is  %d, item num is  %d\n", slab->slab_pages->free_list, slab->item_num);
// }
