#include "../lib.h"
#include "screen.h"
#include "../text_terminal.h"

static unsigned short target_img;
static unsigned char* mem_image = (unsigned char*) 0x0A0000;
static int modex_flag = 0;

int is_modex(){
    return modex_flag;
}

/* VGA register settings for mode X */
static unsigned short mode_X_seq[NUM_SEQUENCER_REGS] = {
    0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};
static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
    0x0008, 0x4109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
    0xFF18
}; 


static unsigned char mode_X_attr[NUM_ATTR_REGS * 2] = {
    0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 
    0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 
    0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 
    0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
    0x10, 0x41, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
    0x14, 0x00, 0x15, 0x00
};

static unsigned short mode_X_graphics[NUM_GRAPHICS_REGS] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
    0xFF08
};

// /* VGA register settings for text mode 3 (color text) */
// static unsigned short text_seq[NUM_SEQUENCER_REGS] = {
//     0x0100, 0x2001, 0x0302, 0x0003, 0x0204
// };
// static unsigned short text_CRTC[NUM_CRTC_REGS] = {
//     0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
//     0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
//     0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
//     0xFF18
// };
// static unsigned char text_attr[NUM_ATTR_REGS * 2] = {
//     0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 
//     0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 
//     0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B, 
//     0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
//     0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
//     0x14, 0x00, 0x15, 0x00
// };
// static unsigned short text_graphics[NUM_GRAPHICS_REGS] = {
//     0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
//     0xFF08
// };


/* local functions--see function headers for details */
static void VGA_blank (int blank_bit);
static void set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
				    unsigned char val);
static void set_CRTC_registers (unsigned short table[NUM_CRTC_REGS]);
static void set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2]);
static void set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS]);
static void fill_palette_mode_x ();
// static void fill_palette_text ();
// static void set_text_mode_3 (int clear_scr);
// static void copy_image (unsigned char* img, unsigned short scr_addr);
//static void copy_status(unsigned char* img, unsigned short scr_addr);
//static void set_status_bar ();






/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
static void
VGA_blank (int blank_bit)
{
    /* 
     * Move blanking bit into position for VGA sequencer register 
     * (index 1). 
     */
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile (
	"movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
	"movw $0x03C4,%%dx                                             ;"
	"outb %%al,(%%dx)                                              ;"
	"incw %%dx                                                     ;"
	"inb (%%dx),%%al         /* Read old value.           */       ;"
	"andb $0xDF,%%al         /* Calculate new value.      */       ;"
	"orl %0,%%eax                                                  ;"
	"outb %%al,(%%dx)        /* Write new value.          */       ;"
	"movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
	"inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
	"movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
	"movb $0x20,%%al                                               ;"
	"outb %%al,(%%dx)                                               "
      : : "g" (blank_bit) : "eax", "edx", "memory");
}


/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
static void
set_seq_regs_and_reset (unsigned short table[NUM_SEQUENCER_REGS],
			unsigned char val)
{
    /* 
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    REP_OUTSW (0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}

    /* Set VGA miscellaneous output register. */
    OUTB (0x03C2, val);

    /* Turn sequencer on (array values above should always force reset). */
    OUTW (0x03C4,0x0300);
}


/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
static void
set_CRTC_registers (unsigned short table[NUM_CRTC_REGS])
{
    /* clear protection bit to enable write access to first few registers */
    OUTW (0x03D4, 0x0011); 
    REP_OUTSW (0x03D4, table, NUM_CRTC_REGS);
}


/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
static void 
set_attr_registers (unsigned char table[NUM_ATTR_REGS * 2])
{
    /* Reset attribute register to write index next rather than data. */
    asm volatile (
	"inb (%%dx),%%al"
      : : "d" (0x03DA) : "eax", "memory");
    REP_OUTSB (0x03C0, table, NUM_ATTR_REGS * 2);
}


/*
 * set_graphics_registers
 *   DESCRIPTION: Set VGA graphics registers.
 *   INPUTS: table -- table of graphics register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */   
static void
set_graphics_registers (unsigned short table[NUM_GRAPHICS_REGS])
{
    REP_OUTSW (0x03CE, table, NUM_GRAPHICS_REGS);
}


/*
 * fill_palette_mode_x
 *   DESCRIPTION: Fill VGA palette with necessary colors for the adventure 
 *                game.  Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */   
static void
fill_palette_mode_x ()
{
    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    /* these are coded for 2 bits red, 2 bits green, 2 bits blue */
    static unsigned char palette_RGB[64][3] = {
	{0x00, 0x00, 0x00}, {0x00, 0x00, 0x15},
	{0x00, 0x00, 0x2A}, {0x00, 0x00, 0x3F},
	{0x00, 0x15, 0x00}, {0x00, 0x15, 0x15},
	{0x00, 0x15, 0x2A}, {0x00, 0x15, 0x3F},
	{0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x15},
	{0x00, 0x2A, 0x2A}, {0x00, 0x2A, 0x3F},
	{0x00, 0x3F, 0x00}, {0x00, 0x3F, 0x15},
	{0x00, 0x3F, 0x2A}, {0x00, 0x3F, 0x3F},
	{0x15, 0x00, 0x00}, {0x15, 0x00, 0x15},
	{0x15, 0x00, 0x2A}, {0x15, 0x00, 0x3F},
	{0x15, 0x15, 0x00}, {0x15, 0x15, 0x15},
	{0x15, 0x15, 0x2A}, {0x15, 0x15, 0x3F},
	{0x15, 0x2A, 0x00}, {0x15, 0x2A, 0x15},
	{0x15, 0x2A, 0x2A}, {0x15, 0x2A, 0x3F},
	{0x15, 0x3F, 0x00}, {0x15, 0x3F, 0x15},
	{0x15, 0x3F, 0x2A}, {0x15, 0x3F, 0x3F},
	{0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x15},
	{0x2A, 0x00, 0x2A}, {0x2A, 0x00, 0x3F},
	{0x2A, 0x15, 0x00}, {0x2A, 0x15, 0x15},
	{0x2A, 0x15, 0x2A}, {0x2A, 0x15, 0x3F},
	{0x2A, 0x2A, 0x00}, {0x2A, 0x2A, 0x15},
	{0x2A, 0x2A, 0x2A}, {0x2A, 0x2A, 0x3F},
	{0x2A, 0x3F, 0x00}, {0x2A, 0x3F, 0x15},
	{0x2A, 0x3F, 0x2A}, {0x2A, 0x3F, 0x3F},
	{0x3F, 0x00, 0x00}, {0x3F, 0x00, 0x15},
	{0x3F, 0x00, 0x2A}, {0x3F, 0x00, 0x3F},
	{0x3F, 0x15, 0x00}, {0x3F, 0x15, 0x15},
	{0x3F, 0x15, 0x2A}, {0x3F, 0x15, 0x3F},
	{0x3F, 0x2A, 0x00}, {0x3F, 0x2A, 0x15},
	{0x3F, 0x2A, 0x2A}, {0x3F, 0x2A, 0x3F},
	{0x3F, 0x3F, 0x00}, {0x3F, 0x3F, 0x15},
	{0x3F, 0x3F, 0x2A}, {0x3F, 0x3F, 0x3F}
    };

    /* Start writing at color 0. */
    OUTB (0x03C8, 0x00);

    /* Write all 64 colors from array. */
    REP_OUTSB (0x03C9, palette_RGB, 64 * 3);
}


/*
 * fill_palette
 *   DESCRIPTION: Fill VGA palette with selected 192 colors.
 *                the 64-256 are written
 *   INPUTS: palette -- a pointer to palatte
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the 64-256 palette colors
 */   
void
fill_palette (const void * palette){
    // Start at 64th color  (0x40). 
    OUTB(0x03C8, 0x40);

    // Write colors from array. 
    REP_OUTSB(0x03C9, palette, 192 * 3);
    // 192 is number of colors to be written in palette, 3 is R G B dimensions	
}

int screen_set_one_palette(unsigned char index, unsigned char r, unsigned char g, unsigned char b){
    unsigned char val[3] = {r, g, b};
    OUTB(0x03C8, index);
    REP_OUTSB(0x03C9, val, 3);
    return 0;
}

int screen_set_palette(unsigned char index, int num, unsigned char* palette){
    OUTB(0x03C8, index);
    REP_OUTSB(0x03C9, palette, 3*num);
    return 0;
}

/*
 * fill_palette_text
 *   DESCRIPTION: Fill VGA palette with default VGA colors.
 *                Only the first 32 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 32 palette colors
 */   
// static void
// fill_palette_text ()
// {
//     /* 6-bit RGB (red, green, blue) values VGA colors and grey scale */
//     static unsigned char palette_RGB[32][3] = {
// 	{0x00, 0x00, 0x00}, {0x00, 0x00, 0x2A},   /* palette 0x00 - 0x0F    */
// 	{0x00, 0x2A, 0x00}, {0x00, 0x2A, 0x2A},   /* basic VGA colors       */
// 	{0x2A, 0x00, 0x00}, {0x2A, 0x00, 0x2A},
// 	{0x2A, 0x15, 0x00}, {0x2A, 0x2A, 0x2A},
// 	{0x15, 0x15, 0x15}, {0x15, 0x15, 0x3F},
// 	{0x15, 0x3F, 0x15}, {0x15, 0x3F, 0x3F},
// 	{0x3F, 0x15, 0x15}, {0x3F, 0x15, 0x3F},
// 	{0x3F, 0x3F, 0x15}, {0x3F, 0x3F, 0x3F},
// 	{0x00, 0x00, 0x00}, {0x05, 0x05, 0x05},   /* palette 0x10 - 0x1F    */
// 	{0x08, 0x08, 0x08}, {0x0B, 0x0B, 0x0B},   /* VGA grey scale         */
// 	{0x0E, 0x0E, 0x0E}, {0x11, 0x11, 0x11},
// 	{0x14, 0x14, 0x14}, {0x18, 0x18, 0x18},
// 	{0x1C, 0x1C, 0x1C}, {0x20, 0x20, 0x20},
// 	{0x24, 0x24, 0x24}, {0x28, 0x28, 0x28},
// 	{0x2D, 0x2D, 0x2D}, {0x32, 0x32, 0x32},
// 	{0x38, 0x38, 0x38}, {0x3F, 0x3F, 0x3F}
//     };

//     /* Start writing at color 0. */
//     OUTB (0x03C8, 0x00);

//     /* Write all 32 colors from array. */
//     REP_OUTSB (0x03C9, palette_RGB, 32 * 3);
// }

void  clear_screens ()
{
    /* Write to all four planes at once. */ 
    SET_WRITE_MASK (0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset (mem_image, 0, MODE_X_MEM_SIZE);
}




int modex_init(){
    /* One display page goes at the start of video memory. */
    target_img = 0x0000;


    VGA_blank (1);                               /* blank the screen  */
    set_seq_regs_and_reset (mode_X_seq, 0x63);   /* sequencer registers   */
    set_CRTC_registers (mode_X_CRTC); /* CRT control registers */

    set_attr_registers (mode_X_attr);  /* attribute registers   */

    set_graphics_registers (mode_X_graphics);    /* graphics registers    */
    fill_palette_mode_x ();			 /* palette colors        */
    clear_screens ();				 /* zero video memory     */
    memset (mem_image + target_img, 20, PLANE_SIZE);
    VGA_blank (0);			         /* unblank the screen    */

    modex_flag = 1;

    /* Return success. */
    return 0;
}




int screen_draw_dot(int x, int y, unsigned char color){
    // parameter check 
    if(x<0||x>=IMAGE_X_DIM){
        kprintf("x out of range"); 
        return -1;
    }

    if(y<0||y>=IMAGE_Y_DIM){
        kprintf("y out of range"); 
        return -1;
    }

    int pixel_addr = y*IMAGE_X_DIM+x;
    int plane = x%4;
    int buf_index = (3-plane)*PLANE_SIZE + pixel_addr/PLANE_NUMBER;
    screen_buf[buf_index] = color;
    return 0; // success 
}


int screen_display(char* buf){
    if(buf==NULL){
        kprintf("buf is NULL");
        return -1;
    }
    target_img ^= 0x4000;
    
    int i;		  /* loop index over video planes        */
    /* Draw to each plane in the video memory. */
    for (i = 0; i < 4; i++) {
        SET_WRITE_MASK (1 << (i + 8));
        memcpy(mem_image+target_img, buf+(3-i)*PLANE_SIZE ,PLANE_SIZE);
    }
    OUTW (0x03D4, (target_img & 0xFF00) | 0x0C);
    OUTW (0x03D4, ((target_img & 0x00FF) << 8) | 0x0D);
    return 0;
} // buf order: 3 2 1 0


// void copy_image(unsigned char* img, unsigned short scr_addr)
// {
//     /* 
//      * memcpy is actually probably good enough here, and is usually
//      * implemented using ISA-specific features like those below,
//      * but the code here provides an example of x86 string moves
//      */
//     asm volatile (
//         "cld                                                 ;"
//        	"movl $1440,%%ecx                                   ;"
//        	"rep movsb    # copy ECX bytes from M[ESI] to M[EDI]  "
//       : /* no outputs */
//       : "S" (img), "D" (mem_image + scr_addr) 
//       : "eax", "ecx", "memory"
//     );
// }

int screen_show(){
    /* 
     * Change the VGA registers to point the top left of the screen
     * to the video memory that we just filled.
     */
    
    return 0;
}





