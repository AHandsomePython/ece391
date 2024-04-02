#include "buddy.h"
#include "../paging.h"
#include "../process.h"
#include "../lib.h"
#include "../terminal.h"

// #define find_level_start_bit(order) ((1<<10)-(1<<(10-order)))
// #define get_start_index(order)  (find_level_start_bit(order)>>5)
// #define get_start_bit(order) (find_level_start_bit(order)&0x1F)
// #define test_free_bit(buddy, n) ((buddy->free_bitmap[(n>>5)] & (1<<(n&0x1F)))!=0)
// #define test_pair_bit(buddy, order, offset) ((buddy->pair_bitmap[((find_level_start_bit(order)+offset)>>5)] & ((1<<(find_level_start_bit(order)+offset))&0x1F))!=0)
// #define flip_free_bit(buddy, n) (buddy->free_bitmap[(n>>5)] ^= (1<<(n&0x1F)))
// #define flip_pair_bit(buddy, order, offset) (buddy->pair_bitmap[((find_level_start_bit(order)+offset)>>5)] ^= ((1<<(find_level_start_bit(order)+offset))&0x1F))

#define get_bit(val, n) ((val >> n) & 0x1)
#define flip_bit(val, h) (val ^= (1 << n))
#define lowbit(val) (val & (-val))
#define pair_bitmap_get(arr, order, n) ((arr[order2start[order] + (n >> 5)] >> (n & 0x1F)) & 0x1)
#define pair_bitmap_flip(arr, order, n) (arr[order2start[order] + (n >> 5)] ^= (1 << (n & 0x1F)))
#define free_bitmap_get(arr, n) ((arr[n >> 5] >> (n & 0x1F)) & 0x1)
#define free_bitmap_flip(arr, n) (arr[n >> 5] ^= (1 << (n & 0x1F)))


static uint8_t order2start[10] = {0, 16, 24, 28, 30, 31, 32, 33, 34, 35};


//flag = 0 means left, flag = 1 means right
int pair_to_free(int order, int n, int flag){
    return (n*2+flag)*(1<<order);
}

int pair_get_child_off(int order, int n, int flag){
    return (n*2+flag);
}

int pair_get_parent_off(int order, int n){
    return n/2;
}

// return 0 for left free, return 1 for right free
// return -1 for not free
// if both free, return 0 as a primary choice
int pair_bit_map_free_test(buddy_t* buddy, int order, int n){
    if (page_free_test(buddy, order, n, 0) == 0){
        return 0;
    }
    else if (page_free_test(buddy, order, n, 1) == 0){
        return 1;
    }
    return -1;
}


int32_t buddy_init(buddy_t* buddy, uint32_t page_off){
    uint32_t i;
    buddy->page_off = page_off;
    for(i=0; i<BUDDY_PAIR_BITMAP_SIZE; i++) buddy->pair_bitmap[i] = 0;
    for(i=0; i<BUDDY_FREE_BITMAP_SIZE; i++) buddy->free_bitmap[i] = 0;
    map_4MB_page(page_off, page_off);
    return 1;
}

/*
allocate and set bitmap 
input: buddy system, order--: 2^order numbers of pages is needed
output: index of the newly allocated page, -1 for fail
side effect: change bit map
*/
int32_t buddy_alloc(buddy_t* buddy, int order){   
    int32_t n;
    int32_t free_map_index;
    int32_t order_temp = order;
    int32_t flag;   // tells whether left or right is free
    // 10 is the maximal number of order
    while(order < 10 && ((n = block_available( buddy, order))==-1) ){
        order++;
    }
    if (n == -1){
        return -1;  // no more space
    }
    // starts at 2^(order+1)*n, first half is 
    // 2^(order+1)*n to 2^(order+1)*n + 2^(order)-1,   offset = 0 to 2^order-1
    // second half, offset is 2^order to 2^(order+1) - 1
    if (page_free_test(buddy, order, n, 0) == 0){   //test left
        flag = 0;
    }
    else {
        flag = 1;
    }
    pair_bitmap_flip(buddy->pair_bitmap, order, n);   // flip pair bit map
    if (order == order_temp){
    	// already at the end 
    	page_free_set(buddy, order, n, flag);
    	free_map_index = pair_to_free(order, n, flag);
    	return free_map_index;
	}
    order--;  // go to previous order
    n = pair_get_child_off(order, n, flag);
    while (order_temp < order){
        pair_bitmap_flip(buddy->pair_bitmap, order, n);
        n = pair_get_child_off(order, n, 0);
        order--;
    }
    pair_bitmap_flip(buddy->pair_bitmap, order, n);
    if ( page_free_test(buddy, order, n, 0)== 0){
    	page_free_set(buddy, order, n, 0);
    	free_map_index = pair_to_free(order, n, 0);
    	return free_map_index;
	}
	else {
		page_free_set(buddy, order, n, 1);
		free_map_index = pair_to_free(order, n, 1);
    	return free_map_index;
	}
    return -1;
}



/*
set free_pages from index 
free_map_index to free_map_index+2^order -1

offset is 0 to 2^order-1 
or 2^order to 2^(order+1) - 1
*/
void page_free_set(buddy_t* buddy, int order, int n, int flag){
    int free_map_index = (n*2+flag)*(1<<order);
    int len = (1<<order);
    int i;
    for (i=0;i<len;i++){
    	//printf("free index------%d\n", (free_map_index+i));
    	//printf("free bitmap------%x\n", buddy->free_bitmap[0]);
        free_bitmap_flip(buddy->free_bitmap, (free_map_index+i));
        //printf("free bitmap------%x\n", buddy->free_bitmap[0]);
    }
    return;
}


/*
test from free_map_index+0 to free_map_index+2^order-1
return 0 for free, -1 for not free 
size of a free page is 2^order
*/
int32_t page_free_test(buddy_t* buddy, int order, int n, int flag){
    int free_map_index = pair_to_free(order, n, flag);
    int len = (1<<order);
    int i;
    for (i=0;i<len;i++){
        if (free_bitmap_get(buddy->free_bitmap, (free_map_index+i)) ){
            return -1;
        }
    }
    return 0;
}


/*
whether the buddy system at that order is available?
if there's a 1 at that block, that's available, then it's not available
for the higjest order block, if it's 0 but two child pages are empty, it's also avialable
return the first offset (for pair bit map) at that order for available
return -1 for not available
return 0 for available
*/
int32_t block_available(buddy_t* buddy, int order){
    int i;
    int len = (1<<(9-order));
    if (order == 9){ // 9 is the greatest order index
        if (pair_bit_map_free_test(buddy, 9, 0) >=0 ){
        	return 0;
		}
        return -1; // the whole is empty
    }
    for (i=0;i<len;i++){
        if (pair_bitmap_get(buddy->pair_bitmap, order, i)){
            return i;   // return the 1's offset of the pair bit map
        }
    }
    return -1;
}



/*
order_available?


block id is the index on free_bit_map

return 0 for success
return -1 for fail

if order = 0, id % 1 = 0
order = 1, id % 2 = 0
order = 2, id % 4 =0

block_id is from 0 to 1023
flip 2^order bits in free bit map
*/
int32_t buddy_free(buddy_t* buddy, uint32_t block_id, int order){
    int n;
    if (block_id % (1<<order) != 0){
        return -1;
    }
    int i;
    for (i=0;i<(1<<order);i++){
        free_bitmap_flip(buddy->free_bitmap, (block_id+i));
    }
    

    // flip the pair bit map
    n = (block_id>>(order+1));  // get off set for pair bit map

    while (pair_bitmap_get(buddy->pair_bitmap, order, n) == 1 && order <3)
    {
        pair_bitmap_flip(buddy->pair_bitmap, order, n);
        order++;
        n = n/2;
    }
    pair_bitmap_flip(buddy->pair_bitmap, order, n);  // flip the end 
    return 0;
}

int malloc_init(){
    // initialize buddy system
    // int i;
    buddy_init(&buddy_sys, MALLOC_ADDR);
    // for (i=0;i<PAGE_NUMS;i++){
    //     ptr_map[i].ptr = NULL;
    //     ptr_map[i].free_map_index = -1;
    //     ptr_map[i].order = -1;
    // }
    return 0;
}


/*
size is in byte -->1
page num is in 4kb -->4*2^10 = 2^12

1111 1111 1111 -> oxfff
*/
void* page_alloc(int order){
    // uint32_t page_num = (size>>12);
    // int order = 0;
    // int i;
    // int flag = 0;
    int32_t free_map_index;
    // if ((size&(0xfff)) != 0){
    //     page_num++;
    // }
    // // size/(2^12)+1/0  if  size%(2^12) != 0
    // // translate it into orders

    // while ((1<<order) < page_num)
    // {
    //     order++;
    // }
    // 2^order >= page_num
    free_map_index = buddy_alloc(&buddy_sys, order);
    if (free_map_index <0){
        printf("cannot malloc more spaces\n");
        return NULL;
    }
    //printf("newly allocated page's index is %d\n", free_map_index);
    // 2^10 pages, size of each page is 2^12 bytes
    // vitural is set equal to physical_addr
    int32_t physical_addr = (free_map_index<<12)+buddy_sys.page_off;
    // for (i=0;i<PAGE_NUMS;i++){
    //     //store at pointer map tabel
    //     if (ptr_map[i].ptr != NULL){
    //         continue;
    //     }
    //     else {
    //         // store data
    //         ptr_map[i].ptr = (void*)physical_addr;
    //         ptr_map[i].order = order;
    //         ptr_map[i].free_map_index = free_map_index;
    //         flag = 1;
    //         break;
    //     }
    // }
    // if (flag == 0){
    //     printf("cannot malloc more spaces\n");
    // }
    // set the 4mb page
    // map_4MB_page(physical_addr,physical_addr);
    return (void*)physical_addr;
}

void page_free(void* ptr, int order){
    if (ptr == NULL){
        printf("cannot free NULL pointer\n");
        return;
    }
    //search map
    // int i;
    // int flag = 0;
    uint32_t free_map_index = free_map_index_get(ptr, &buddy_sys);
    //printf("free 1 %d\n", free_map_index);
    buddy_free(&buddy_sys, free_map_index, order);
    //printf("free %d\n", free_map_index);
    return;
}

uint32_t free_map_index_get(void* ptr, buddy_t* buddy){
    return (((((uint32_t)ptr)&0xfffff000)- (uint32_t)(buddy->page_off))>>12);
}

