#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int share = 0;

void thread_1(){
    while(1){
        ece391_printf((uint8_t*)"hello_thread_01  %d\n", share++);
    }
}

int main(){
    ece391_thread_create(thread_1);
    while(1){
        ece391_printf((uint8_t*)"hello_thread_02  %d\n", share++);
    }
}
