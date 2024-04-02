#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "keyboard.h"

#define TERMINAL_INPUT_BUF_SIZE 128

#define TERMINAL_NUM 3

struct terminal_ops;
typedef struct terminal_ops terminal_ops_t;

#define TERMINAL_DEFULT_TEXT_COLOR 0x3C
#define TERMINAL_DEFULT_BACKGROUND_COLOR 0x03
#define TERMINAL_TEXT_BUF_SIZE 2100
typedef struct terminal{
    int start_row_index;
    int cur_x;
    int cur_y;
    unsigned char text_color;
    unsigned char background_color;
    terminal_ops_t* ops;    // can not be modified after driver init
    char* text_buf;         // can not be modified after driver init
    int input_buf_cur_pos;
    char* input_buf;        // can not be modified after driver init
    void* occupied;         // Used for GUI terminal to store the pointer to window
} terminal_t;

struct terminal_ops{
    int (*init) (terminal_t* terminal);     // Before calling the init function, you must set the occupied field!
    int (*putc) (terminal_t* terminal, char c);
    int (*delc) (terminal_t* terminal);
    int (*clear) (terminal_t* terminal);
    int (*puts) (terminal_t* terminal, char* s);
    int (*printf) (terminal_t* terminal, char* format, ...);
    int (*show) (terminal_t* terminal);
    int (*exit) (terminal_t* terminal);
};

// Terminal Driver Init
int terminal_driver_init();     // terminal driver init function

// Terminal Allocation Interface
terminal_t* terminal_alloc();    // dynamic terminal allocator
int terminal_free(terminal_t* terminal);    // free the dynamic allocated terminal
terminal_t* terminal_get(int index);
int terminal_get_index(terminal_t* terminal);

// Terminal System Call Interface
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);

// Terminal & Keyboard Interface
int terminal_keyb_handler(keyb_status_t* keyboard);
void ctrl_l_handler(terminal_t* terminal);
void ctrl_c_handler(terminal_t* terminal);
void enter_handler(terminal_t* terminal);
void bsp_handler(terminal_t* terminal);
void terminal_buf_put(terminal_t* terminal, char c);
void terminal_buf_delete(terminal_t* terminal);
void terminal_buf_clear(terminal_t* terminal);

// Kernel Interface
int kputc(char c);
int kdelc();
int kclear();
int kputs(char* s);
int kprintf(char* format, ...);

#endif
