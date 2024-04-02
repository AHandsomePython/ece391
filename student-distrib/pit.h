#ifndef PIT_H
#define PIT_H

#define PIT_IN_FRQ     1193181.6666 // input frq
#define PIT_MIN_FRQ    18.206785 // minimum output frq for pit
#define CH0_DATA_PORT  0x40
#define MODE_REG       0x43  // mode reg port
#define MODE3          0x36
// 0x36 =  0  0  1  1  0  1  1  0
// first 00 means chanel 0, 11 means using both high 8 bits and low 8 bits
// as count, 011 means using mode 3 (square wave mode), last 0 means binary mode
#define PIT_IRQ        0



int pit_init(int f);
int pit_handler(unsigned int ignore);
int pit_set_frq(int f);



#endif
