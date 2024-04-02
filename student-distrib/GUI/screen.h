#ifndef SCREEN_H
#define SCREEN_H

#include "../lib.h"

#define IMAGE_X_DIM     320   /* pixels; must be divisible by 4             */
#define IMAGE_Y_DIM     200   /* pixels                                     */
#define IMAGE_X_WIDTH   (IMAGE_X_DIM / 4)          /* addresses (bytes)     */
#define PLANE_SIZE      (IMAGE_X_WIDTH * IMAGE_Y_DIM)
#define IMAGE_PLANE_SIZE (IMAGE_X_WIDTH * IMAGE_Y_DIM)
#define PALETTE_SIZE    256
#define PLANE_NUMBER    4

#define VID_MEM_SIZE       131072
#define MODE_X_MEM_SIZE     65536
#define NUM_SEQUENCER_REGS      5
#define NUM_CRTC_REGS          25
#define NUM_GRAPHICS_REGS       9
#define NUM_ATTR_REGS          22


#define SET_WRITE_MASK(mask_hi_bits)                                    \
do {                                                                    \
    asm volatile ("                                                     \
	movw $0x03C4,%%dx    	/* set write mask                    */;\
	movb $0x02,%b0                                                 ;\
	outw %w0,(%%dx)                                                 \
    " : : "a" ((mask_hi_bits)) : "edx", "memory");                      \
} while (0)

/* macro used to write a byte to a port */
#define OUTB(port,val)                                                  \
do {                                                                    \
    asm volatile ("                                                     \
        outb %b1,(%w0)                                                  \
    " : /* no outputs */                                                \
      : "d" ((port)), "a" ((val))                                       \
      : "memory", "cc");                                                \
} while (0)

/* macro used to write two bytes to two consecutive ports */
#define OUTW(port,val)                                                  \
do {                                                                    \
    asm volatile ("                                                     \
        outw %w1,(%w0)                                                  \
    " : /* no outputs */                                                \
      : "d" ((port)), "a" ((val))                                       \
      : "memory", "cc");                                                \
} while (0)

/* 
 * macro used to write an array of two-byte values to two consecutive ports 
 */
#define REP_OUTSW(port,source,count)                                    \
do {                                                                    \
    asm volatile ("                                                     \
     1: movw 0(%1),%%ax                                                ;\
	outw %%ax,(%w2)                                                ;\
	addl $2,%1                                                     ;\
	decl %0                                                        ;\
	jne 1b                                                          \
    " : /* no outputs */                                                \
      : "c" ((count)), "S" ((source)), "d" ((port))                     \
      : "eax", "memory", "cc");                                         \
} while (0)

/* 
 * macro used to write an array of one-byte values to two consecutive ports 
 */
#define REP_OUTSB(port,source,count)                                    \
do {                                                                    \
    asm volatile ("                                                     \
     1: movb 0(%1),%%al                                                ;\
	outb %%al,(%w2)                                                ;\
	incl %1                                                        ;\
	decl %0                                                        ;\
	jne 1b                                                          \
    " : /* no outputs */                                                \
      : "c" ((count)), "S" ((source)), "d" ((port))                     \
      : "eax", "memory", "cc");                                         \
} while (0)



char screen_buf[IMAGE_X_DIM*IMAGE_Y_DIM];

// Init functions
int modex_init();
int text_mode_init();

// ModeX screen opertion interface
int is_modex();
int screen_draw_dot(int x, int y, unsigned char color);
int screen_show();
int screen_display(char* buf); // buf order: 3 2 1 0
int screen_set_one_palette(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
int screen_set_palette(unsigned char index, int num, unsigned char* palette);

#endif
