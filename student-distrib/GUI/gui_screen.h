#ifndef GUI_SCREEN_H
#define GUI_SCREEN_H

#include "screen.h"
#include "window.h"
#include "gui_color.h"

#define FONT_WIDTH   8
#define FONT_HEIGHT 16

unsigned char* get_desktop_img();
unsigned char* get_background_img();
void* get_font_data();

//
// GUI Desktop Img Functions
//
#define DESKTOP_STATUS_BAR_HEIGHT 16
#define DESKTOP_STATUS_BAR_COLOR STATUS_BAR_COLOR
#define DESKTOP_STATUS_BAR_SPLIT_COLOR STATUS_BAR_TEXT_COLOR
#define DESKTOP_STATUS_BAR_NUM 6
#define STATUS_BAR_BLOCK_START_X 18
#define STATUS_BAR_BLOCK_WIDTH ((IMAGE_X_DIM - STATUS_BAR_BLOCK_START_X) / DESKTOP_STATUS_BAR_NUM)
int GUI_build_background(unsigned char* img);
int GUI_desktop_init();

//
// Status Bar functions
//
int GUI_status_bar_draw_font_defult_size(int cur_x, char c, unsigned char color);
int GUI_status_bar_draw_dot(int x, int y, unsigned char color);

//
// GUI Screen Class
//
unsigned char cur_screen_buf[IMAGE_Y_DIM * IMAGE_X_DIM];  // Plane 3,2,1,0
int GUI_screen_init();
int GUI_screen_draw_dot(int x, int y, unsigned char color);
int GUI_screen_clear();
int GUI_screen_clear_without_show();
int GUI_screen_show();  // Copy screen buf to the vidmem
int GUI_screen_draw_dot_on_buf(int x, int y, unsigned char color, unsigned char* buf);
int GUI_screen_draw_line(int x_start, int y_start, int x_end, int y_end, unsigned char color);
int GUI_screen_draw_rect(int x, int y ,int w, int h, unsigned char color);
int plane_buf_draw_dot(unsigned char* buf, int x, int y, unsigned char color);

#endif
