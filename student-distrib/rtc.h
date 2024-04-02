#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "spinlock.h"
#include "filesys.h"


// index for register
#define RTC_REG_A 0xA
#define RTC_REG_B 0xB
#define RTC_REG_C 0xC
// port number
#define RTC_PORT  0x70
#define RTC_CMOS_PORT  0x71
// non-maskable_interrupt index
#define RTC_NMI 0x80
// used to enable irq
#define RTC_ENABLE_INTR 0x40
// max interrupt rate
#define INTERRUPT_RATE 3
// used to clear low 4 bits for new interrupt rate
#define BIT_MASK 0xF0
// rtc is in irq 8
#define RTC_IRQ 8

#define INT32_BYTE 4


struct rtc_lock
{
    unsigned int flag;
    spinlock_t lock;
};

typedef struct rtc_buf{
    unsigned int freq;
    int (*handler)(struct rtc_buf* this);
}rtc_buf_t;

// initialize RTC device. It firstly turn on IRQ 8, and then change the interrupt rate 
void rtc_init();
// interrupt handle for RTC
int rtc_interrupt(unsigned int ignore);
int test_power2(int f);
int rtc_frequency(int f);



int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

#endif

