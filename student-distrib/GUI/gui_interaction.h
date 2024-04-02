#ifndef GUI_INTERACTION_H
#define GUI_INTERACTION_H

//
// Widget
//
typedef struct GUI_widget GUI_widget_t;
struct GUI_widget{
    // Double linked list linkage
    GUI_widget_t* next;
    GUI_widget_t* prev;
    // Status
    int status;
    // Graphic info
    int x;
    int y;
    int w;
    int h;
    // Functions
    int (*constructor) (GUI_widget_t* widget);
    int (*deconstructor) (GUI_widget_t* widget);
    int (*act) (GUI_widget_t* widget, int x, int y);
    int (*show) (GUI_widget_t* widget);
};
int widget_add(GUI_widget_t* widget);
int widget_del(GUI_widget_t* widget);
int widget_build();
int widget_act(int x, int y);
int widget_clear();
// Widget wrapper functions
int widget_driver_init();
int GUI_widget_handler(int x, int y);
int GUI_widget_right_handler(int x, int y);
// int GUI_widget_start();



//
// 1. Palette Menu
//
int palette_menu_constructor(GUI_widget_t* widget);
int palette_menu_act(GUI_widget_t* widget, int x, int y);
int palette_menu_show(GUI_widget_t* widget);

//
// 2. Color Rolling Bar
//
int color_rolling_bar_constructor(GUI_widget_t* widget);
int color_rolling_bar_act(GUI_widget_t* widget, int x, int y);
int color_rolling_bar_show(GUI_widget_t* widget);

//
// 3. Start Menu
//
int start_menu_constructor(GUI_widget_t* widget);
int start_menu_act(GUI_widget_t* widget, int x, int y);
int start_menu_show(GUI_widget_t* widget);

//
// 4. Right Button Menu
//
int right_button_menu_constructor(GUI_widget_t* widget);
int right_button_menu_act(GUI_widget_t* widget, int x, int y);
int right_button_menu_show(GUI_widget_t* widget);

//
// 5. Sound Bar
//
// int sound_bar_constructor(GUI_widget_t* widget);
// int sound_bar_act(GUI_widget_t* widget, int x, int y);
// int sound_bar_show(GUI_widget_t* widget);

#endif
