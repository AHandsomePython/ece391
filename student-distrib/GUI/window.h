#ifndef WINDOW_H
#define WINDOW_H

#include "../lib.h"
#include "screen.h"
#include "gui_color.h"
// #include "gui.h"



//
// GUI Window Class
//
#define WINDOW_NAME_SIZE 30
#define WINDOW_DEFULT_WIDTH     (IMAGE_X_DIM * 15 / 16)
#define WINDOW_DEFULT_HEIGHT    (IMAGE_Y_DIM * 15 / 16)
#define WINDOW_DEFULT_X     0
#define WINDOW_DEFULT_Y     0
#define WINDOW_BAR_HEIGHT   12
#define WINDOW_BAR_SIDE_GAP 4
#define WINDOW_BAR_BACKGROUND_COLOR WINDOW_BAR_COLOR
#define WINDOW_BAR_TEXT_COLOR WINDOW_TEXT_COLOR
#define WINDOW_BAR_BUTTON1_COLOR BUTTON1_COLOR
#define WINDOW_BAR_BUTTON2_COLOR BUTTON2_COLOR
#define WINDOW_BAR_BUTTON3_COLOR BUTTON3_COLOR
#define WINDOW_BAR_BUTTON_WIDTH 15
#define WINDOW_BAR_BUTTON_HEIGHT 10
// #define get_button_start_x(window) (window->Wx + 4*window->Ww/5)
#define get_button_start_x(window) (window->Wx + window->Ww - 3*WINDOW_BAR_BUTTON_WIDTH - 10)
#define get_button_start_y(window) (window->Wy)
#define WINDOW_MAX_NUM 6

typedef struct GUI_window{
    int Wx, Wy, Ww, Wh;
    int Px, Py, Pw, Ph;
    int owner_pid;
    int is_show;
    char name[WINDOW_NAME_SIZE + 1];
    // unsigned char* buf;
    unsigned char buf[IMAGE_X_DIM * IMAGE_Y_DIM];
} GUI_window_t;

int GUI_window_init(GUI_window_t* window, const char* name, int owner_pid);
int GUI_window_draw_dot(GUI_window_t* window, int x, int y, unsigned char color);
int GUI_window_draw_font(GUI_window_t* window, int x, int y, int w, int h, char c, unsigned char color);
int GUI_window_draw_font_over(GUI_window_t* window, int x, int y, int w, int h, char c, unsigned char color, unsigned char background);
int GUI_window_change_pos(GUI_window_t* window, int Wx, int Wy, int Ww, int Wh);
int GUI_window_clear(GUI_window_t* window, unsigned char color);
int GUI_window_show(GUI_window_t* window);  // Copy window buf to screen buf
int GUI_window_exit(GUI_window_t* window);

GUI_window_t* GUI_window_alloc();
int GUI_window_free(GUI_window_t* window);
GUI_window_t* GUI_window_get(int index);
int32_t GUI_window_get_index(GUI_window_t* window);

int GUI_window_open(const uint8_t* filename);
int GUI_window_close(int32_t fd);
int GUI_window_read(int32_t fd, void* buf, int32_t nbytes);
int GUI_window_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
