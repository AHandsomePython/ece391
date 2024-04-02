// reference: 
// https://wiki.osdev.org/%228042%22_PS/2_Controller 
// https://wiki.osdev.org/PS/2 
// https://wiki.osdev.org/PS/2_Keyboard 
// http://www.osdever.net/bkerndev/Docs/keyboard.htm 
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "lib.h"
#include "spinlock.h"
// keyb is short write for keyboard
// cmd is short right for command
#define KEYB_DATA_PORT 0x60 
#define KEYB_CMD_PORT 0x64 
#define STATUS_SIZE 4       // 4 states: none, shift, cap, shift and cap
#define SCANCODE_SIZE 128   // 128 keys

// scan code for special pressed key 
// BSP is short-write for backspace 
#define RIGHT_SHIFT_CODE 0x36
#define LEFT_SHIFT_CODE 0x2A
#define CAPS_CODE 0x3A
#define CTRL_CODE 0x1D
#define ALT_CODE 0x38
#define ENTER_CODE 0x1C
#define BSP_CODE 0x0E
#define L_CODE 0x26
#define F1_CODE 0x3B
#define F2_CODE 0x3C
#define F3_CODE 0x3D

// released code
#define LEFT_SHIFT_RELEASED_CODE 0xAA
#define RIGHT_SHIFT_RELEASED_CODE 0xB6
#define ALT_RELEASED_CODE 0xB8
#define CTRL_RELEASED_CODE 0x9D	


#define keyb_irq 0x1




#define KEYBOARD_BUF_SIZE 129 


// struct to record keyboard status 
typedef struct keyb_status{
    unsigned char shift;   // 1 when pressed, 0 otherwise 
    unsigned char cap_lock;  // 1 when cap-lock is set, 0 otherwise
    unsigned char ctrl;     // 1 when ctrl, 0 otherwise
    unsigned char alt;    // 1 when alt, 0 otherwise
    unsigned char current_char;  //ASCII of current pressed key
    unsigned char bsp_flag;
    spinlock_t lock;
}keyb_status_t;


// struct for keyboard buffer, which is used to store keys
typedef struct keyb_buf{
    unsigned int cur_size;
    unsigned int head;
    unsigned int tail;
    char buf[KEYBOARD_BUF_SIZE];
    char current_char;
    spinlock_t lock;
    unsigned char shift;   // 1 when pressed, 0 otherwise 
    unsigned char cap_lock;  // 1 when cap-lock is set, 0 otherwise
    unsigned char ctrl;     // 1 when ctrl, 0 otherwise
    unsigned char alt;    // 1 when alt, 0 otherwise
    unsigned char bsp_flag;
    int (*handler)(struct keyb_buf* this);
}keyb_buf_t;


void swich_terminal(unsigned char scancode);


// set a buffer for keyboard
int keyboard_set_buf(keyb_buf_t* buf_ptr);
// initialize buffer
void keyboard_init();
//handle keyboard interrupt
int keyboard_interrupt(unsigned int ignore);
// put a char into a buffer
int keyboard_buf_put(keyb_buf_t* buf_ptr, char c);
// get a char from buffer
char keyboard_buf_get(keyb_buf_t* buf_ptr);
// read a char from buffer
char keyboard_buf_seek(keyb_buf_t* buf_ptr);
// initiailze keyboard buffer
inline int keyboard_buf_init(keyb_buf_t* buf_ptr);
// set current buffer pointer
int keyboard_set_buf(keyb_buf_t* buf_ptr);
// set currrent buffer to default one
void keyboard_set_default_buffer();
// get a char from current buffer
char get_char();
// read a char from current buffer 
char seek_char();
//keyb_buf_clear
void keyb_buf_clear();
//default_bsp_handler
void default_bsp_handler();
// keyboard_write
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes);
//keyboard_open
int32_t keyboard_open(const uint8_t *filename);
//keyboard_close
int32_t keyboard_close(int32_t fd);
//keyboard_read
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes);

#endif
