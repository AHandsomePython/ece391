#ifndef GUI_H
#define GUI_H

#include "window.h"
#include "../process.h"
#include "mouse.h"

//
// GUI helper functions:
//
#define GUI_MOUSE_DEFULT_COLOR 0x0F
#define GUI_MOUSE_HOLD_COLOR 0x0A
#define GUI_MOUSE_EDGE_COLOR 0x13
int GUI_mouse_handler(mouse_t mouse);
unsigned char get_mouse_color();
int is_in_range(int x, int y, int xs, int ys, int w, int h);

//
// GUI window wrapper functions:
//
int GUI_init();

//
// GUI Status Bar functions
//
int status_bar_add(GUI_window_t* window);
int status_bar_del(GUI_window_t* window);
GUI_window_t* status_bar_get(int index);
GUI_window_t* status_bar_get_from_x(int x);
int status_bar_del_from_x(int x);
void status_bar_add_with_rebuild(GUI_window_t* window);

//
// GUI window relation handlers:
//
int32_t window_list_init();
int32_t window_list_add_head(GUI_window_t* window);
int32_t window_list_add_tail(GUI_window_t* window);
int32_t window_list_delete(GUI_window_t* window);
int32_t window_list_move_to_top(GUI_window_t* window);
int32_t window_list_move_to_bottom(GUI_window_t* window);
int32_t window_list_build_screen();
// Operating Interfaces:
int32_t window_switch(GUI_window_t* window);    // switch the window to the top 
int32_t window_remove(GUI_window_t* window);    // remove this window
GUI_window_t* window_get_top();     // get the top window
GUI_window_t* get_window_from_screen_coordinate(int x, int y);  //get the window at (x, y) on screen

#endif
