#ifndef GUI_TERMINAL_H
#define GUI_TERMINAL_H
#include "../terminal.h"
#include "screen.h"
#include "gui_color.h"

#define GUI_TERMINAL_CHAR_W 8
#define GUI_TERMINAL_CHAR_H 16
#define GUI_TERMINAL_NUM_X  ((int)IMAGE_X_DIM / GUI_TERMINAL_CHAR_W) 
#define GUI_TERMINAL_NUM_Y  ((int)IMAGE_Y_DIM / GUI_TERMINAL_CHAR_H)
#define GUI_TERMINAL_DEFULT_CHAR_COLOR TERMINAL_TEXT_COLOR
#define GUI_TERMINAL_DEFULT_BACKGROUND_COLOR TERMINAL_COLOR

int GUI_terminal_init (terminal_t* terminal);
int GUI_terminal_putc (terminal_t* terminal, char c);
int GUI_terminal_delc (terminal_t* terminal);
int GUI_terminal_clear (terminal_t* terminal);
int GUI_terminal_puts (terminal_t* terminal, char* s);
int GUI_terminal_printf (terminal_t* terminal, char* format, ...);
int GUI_terminal_show (terminal_t* terminal);
int GUI_terminal_exit (terminal_t* terminal);

#endif
