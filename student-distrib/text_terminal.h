#ifndef TEXT_TERMINAL_H
#define TEXT_TERMINAL_H

#include "keyboard.h"
#include "lib.h"
#include "GUI/window.h"
#include "terminal.h"
#include "paging.h"

#define TEXT_TERMINAL_COLS    80
#define TEXT_TERMINAL_ROWS    25

#define VGA_CMD 0x3d4
#define VGA_DATA 0x3d5

#define VGA_TEXT_BUF_ADDR (0xB8000)
// #define VGA_TEXT_BUF_ADDR1 (0xB8000+(80*25*2+160))
// #define VGA_TEXT_BUF_ADDR2 (0xB8000+(80*25*2+160)*2)
#define VGA_TEXT_BUF_ADDR1 (0xBA000)
#define VGA_TEXT_BUF_ADDR2 (0xBC000)


// leave some space

void vga_write_set(int index);
void vga_show_set(int index);
inline int vga_print(int x, int y, char c, unsigned char color);
inline int vga_set_cursor(int index, int x, int y);
inline int vga_up_shift();
int get_addr(int index);



int text_terminal_init(terminal_t* terminal);
int text_terminal_putc(terminal_t* terminal, char c);
int text_terminal_delc(terminal_t* terminal);
int text_terminal_clear (terminal_t* terminal);
int text_terminal_puts (terminal_t* terminal, char* s);
int text_terminal_printf (terminal_t* terminal, char* format, ...);
int text_terminal_show (terminal_t* terminal);
int text_terminal_exit (terminal_t* terminal);





// typedef struct screen{
//     // char buf[SCREEN_ROWS][SCREEN_COLS];
//     char start_row_index;
//     unsigned int cur_x;
//     unsigned int cur_y;
//     unsigned char text_color;
//     unsigned int shift_flag;
// }screen_t;
// screen_t screen;
// keyb_buf_t terminal_buffer;
// //screen_init
// int screen_init(screen_t* screen);
// //screen_putc
// int screen_putc(screen_t* screen, char c);
// // screen_delc
// int screen_delc(screen_t* screen);
// // screen_clear
// int screen_clear(screen_t* screen);
// // screen_puts
// int screen_puts(screen_t* screen, char* str);

// int32_t screen_printf(screen_t* screen, int8_t *format, ...);
// // typedef struct terminal{
// //     keyb_buf_t keyb_buf;
// // }terminal_t;
// // vga_print 
// inline int vga_print(int x, int y, char c, unsigned char color);
// // vga_up_shift
// inline int vga_up_shift();

// //
// //  Terminal Functions:
// //

// // typedef struct terminal{
// //     int is_GUI;
// //     // union{
// //     //     text_terminal_t* text_terminal;
// //     //     GUI_terminal_t* GUI_terminal;
// //     // };
// // } terminal_t;
// #define TEXT_TERMINAL_NUM_X 100
// #define TEXT_TERMINAL_NUM_Y 100
// #define TERMINAL_NUM 3
// typedef struct text_terminal{
//     int start_row_index;
//     int cur_x;
//     int cur_y;
//     unsigned char text_color;
//     unsigned char background_color;
//     char text_buf[TEXT_TERMINAL_NUM_X * TEXT_TERMINAL_NUM_Y];
// } text_terminal_t;

// // text_terminal_t text_terminal_arr[TERMINAL_NUM];
// // int text_terminal_arr[TERMINAL_NUM];

// // ctrl_l_handler
// void ctrl_l_handler();
// // enter_handle
// void enter_handler();
// // bsp_handler
// void bsp_handler();
// // terminal_clear
// void terminal_buf_clear();
// // terminal_ini
// void terminal_init();
// //  terminal_delete
// int terminal_delete();
// // terminal_keyb_handler
// // int terminal_keyb_handler(struct keyb_buf* this);
// //int terminal_keyb_handler(struct keyb_buf* this);
// // terminal_open
// int32_t terminal_open(const uint8_t* filename);
// // terminal_close
// int32_t terminal_close(int32_t fd);
// // terminal_read
// int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
// // terminal_write
// int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);

#endif
