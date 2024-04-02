#ifndef MALLOC_H
#define MALLOC_H
#include "../lib.h"

int32_t malloc_driver_init();
uint32_t get_power_of_two(uint32_t val);
void* kmalloc(uint32_t size);
int32_t kfree(void* ptr);

#endif
