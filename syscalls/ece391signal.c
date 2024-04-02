#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main(){
    ece391_sigsend(0,3);
    return 0;
}   
