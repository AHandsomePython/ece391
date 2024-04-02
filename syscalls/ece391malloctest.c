#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define NULL 0

typedef struct node{
    struct node* next;
    int num;
}node_t;

node_t* start = NULL;

int push(int num){
    node_t* ptr = ece391_malloc(sizeof(node_t));
    if(ptr==NULL) return -1;
    ptr->next = start;
    ptr->num = num;
    start = ptr;
    return 1;
}

int pop(){
    if(start==NULL) return -1;
    node_t* ptr = start;
    int num;
    start = ptr->next;
    num = ptr->num;
    ece391_free(ptr);
    return num;
}

int print(){
    node_t* ptr;
    int cnt = 0;
    for(ptr=start; ptr!=NULL; ptr=ptr->next){
        ece391_printf((int8_t*)"%d ", ptr->num);
        cnt++;
    }
    ece391_printf((int8_t*)"\nTotal Num = %d\n", cnt);
    return cnt;
}

int main(){
    int i=0;
    for(i=0; i<10000; i++){
        push(i);
    }
    for(i=0; i<9990; i++){
        pop();
    }
    for(i=0; i<20; i++){
        push(i+100);
    }
    print();
    for(i=0; i<100; i++){
        pop();
    }
    return 0;
}
