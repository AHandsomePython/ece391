#include "pit.h"
#include "lib.h"
#include "interrupt.h"
#include "scheduling.h"

int pit_init(int f){
    outb(MODE3,MODE_REG);
    if(pit_set_frq(f)==0){
        request_irq(PIT_IRQ, pit_handler, 0);
        enable_irq(PIT_IRQ);
        return 0;
    }
    else{
        return -1;
    }
    
}

int pit_set_frq(int f){
    if(f<0||f>PIT_IN_FRQ){
        return -1;
    }
    if(f<PIT_MIN_FRQ) f = PIT_MIN_FRQ+1;

    uint32_t count = PIT_IN_FRQ / f;
    uint8_t low = (uint8_t) count;
    uint8_t high = (uint8_t) (count>>8); 
    outb(low,CH0_DATA_PORT);
    outb(high,CH0_DATA_PORT);
    return 0;
}


// static int pit_counter = 0;
int pit_handler(unsigned int ignore){
// 进行进程切换
    // if(is_PCB_legal(current)){
    //     pit_counter++;
    //     if(pit_counter==10){
    //         pit_counter = 0;
    //         kputs("pit  ");
    //     }
    // }
    context_switch_rr();
    return 0;
}

