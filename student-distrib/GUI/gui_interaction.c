#include "gui_interaction.h"
#include "gui_screen.h"
#include "gui.h"
#include "gui_color.h"

//
// Widget Structs Define
//
#define PALT_MENU_X_DIM 128
#define PALT_MENU_Y_DIM 128
GUI_widget_t palette_menu = {
    .next = NULL,
    .prev = NULL,
    .status = 0,
    .x = 0,
    .y = IMAGE_Y_DIM - PALT_MENU_Y_DIM,
    .w = PALT_MENU_X_DIM,
    .h = PALT_MENU_Y_DIM,
    .constructor = palette_menu_constructor,
    .deconstructor = NULL,
    .act = palette_menu_act,
    .show = palette_menu_show
};

#define COLOR_ROLLING_BAR_W 96
#define COLOR_ROLLING_BAR_H 32
GUI_widget_t color_rolling_bar = {
    .next = NULL,
    .prev = NULL,
    .status = 0,
    .x = 0,
    .y = 0,
    .w = COLOR_ROLLING_BAR_W,
    .h = COLOR_ROLLING_BAR_H,
    .constructor = color_rolling_bar_constructor,
    .deconstructor = NULL,
    .act = color_rolling_bar_act,
    .show = color_rolling_bar_show
};

#define START_MENU_W 150 
#define START_MENU_H 160
GUI_widget_t start_menu = {
    .next = NULL,
    .prev = NULL,
    .status = 0,
    .x = 0,
    .y = IMAGE_Y_DIM - START_MENU_H,
    .w = START_MENU_W,
    .h = START_MENU_H,
    .constructor = start_menu_constructor,
    .deconstructor = NULL,
    .act = start_menu_act,
    .show = start_menu_show
};

#define RIGHT_BUTTON_MENU_W 120
#define RIGHT_BUTTON_MENU_H 60
GUI_widget_t right_button_menu = {
    .next = NULL,
    .prev = NULL,
    .status = 0,
    .x = 0,
    .y = 0,
    .w = RIGHT_BUTTON_MENU_W,
    .h = RIGHT_BUTTON_MENU_H,
    .constructor = right_button_menu_constructor,
    .deconstructor = NULL,
    .act = right_button_menu_act,
    .show = right_button_menu_show
};

// #define SOUND_BAR_W 50
// #define SOUND_BAR_H 120
// GUI_widget_t sound_bar = {
//     .next = NULL,
//     .prev = NULL,
//     .status = 0,
//     .x = 0,
//     .y = 0,
//     .w = SOUND_BAR_W,
//     .h = SOUND_BAR_H,
//     .constructor = sound_bar_constructor,
//     .deconstructor = NULL,
//     .act = sound_bar_act,
//     .show = sound_bar_show
// };

//
// Helper functions
//
static int draw_dot(unsigned char* buf, int x, int y, int W, int H, unsigned char color){
    if(x<0 || x>=W || y<0 || y>=H) return -1;
    buf[y*W+x] = color;
    return 0;
}

static int draw_rect(unsigned char* buf, int x, int y, int w, int h, int W, int H, unsigned char color){
    int i, j;
    for(i=y; i<y+h; i++){
        for(j=x; j<x+w; j++){
            draw_dot(buf, j, i, W, H, color);
        }
    }
    return 0;
}

static int draw_char(unsigned char* buf, int x, int y, int c, int W, int H, unsigned char color){
    unsigned char (*font)[16] = get_font_data();
    int i, j;
    for(i=0; i<12; i++){
        for(j=0; j<6; j++){
            if((font[(unsigned char)c][i*16/12+1] >> (FONT_WIDTH - j*8/6 - 1)) & 0x1){
                draw_dot(buf, x+j, y+i, W, H, color);
            }
        }
    }
    return 0;
}

static int draw_str(unsigned char* buf, int x, int y, char* str, int W, int H, unsigned char color){
    if(str==NULL) return -1;
    int i = 0;
    while(*str!=0){
        draw_char(buf, x+i*6, y, *str, W, H, color);
        str++;
        i++;
    }
    return 0;
}

//
// Widget
//
static GUI_widget_t widget_dummy;
int widget_driver_init(){
    widget_dummy.next = &widget_dummy;
    widget_dummy.prev = &widget_dummy;
    widget_dummy.x = 0;
    widget_dummy.y = 0;
    widget_dummy.w = 0;
    widget_dummy.h = 0;
    widget_dummy.constructor = NULL;
    widget_dummy.deconstructor = NULL;
    widget_dummy.act = NULL;
    widget_dummy.show = NULL;
    return 0;
}

int widget_add(GUI_widget_t* widget){
    if(widget==NULL) return -1;
    if(widget->status == 1){
        widget_del(widget);
    }
    widget->next = widget_dummy.next;
    widget->prev = &widget_dummy;
    widget->next->prev = widget;
    widget->prev->next = widget;
    widget->status = 1;
    if(widget->constructor!=NULL){
        widget->constructor(widget);
    }
    return 0;
}

int widget_del(GUI_widget_t* widget){
    if(widget==NULL || widget->next==NULL || widget->prev==NULL) return -1;
    if(widget->status == 0) return -1;
    widget->next->prev = widget->prev;
    widget->prev->next = widget->next;
    widget->status = 0;
    if(widget->deconstructor!=NULL){
        widget->deconstructor(widget);
    }
    return 0;
}

int widget_build(){
    GUI_widget_t* ptr;
    for(ptr=widget_dummy.prev; ptr!=&widget_dummy; ptr=ptr->prev){
        if(ptr->show!=NULL){
            ptr->show(ptr);
        }
    }
    return 0;
}

int widget_act(int x, int y){
    GUI_widget_t* ptr;
    int ret_val = 0;
    for(ptr=widget_dummy.next; ptr!=&widget_dummy; ptr=ptr->next){
        if(is_in_range(x, y, ptr->x, ptr->y, ptr->w, ptr->h)){
            if(ptr->act!=NULL){
                ptr->act(ptr, x, y);
            }
            ret_val = 1;
            break;
        }
    }
    return ret_val;
}

int widget_clear_no_show(){
    while(widget_dummy.next!=&widget_dummy){
        widget_del(widget_dummy.next);
    }
    return 0;
}

int widget_clear(){
    while(widget_dummy.next!=&widget_dummy){
        widget_del(widget_dummy.next);
    }
    window_list_build_screen();
    GUI_screen_show();
    return 0;
}



//
// Widget Mouse Handler
//
int GUI_widget_handler(int x, int y){
    int ret_val = 0;
    if(is_in_range(x, y, 0, IMAGE_Y_DIM-DESKTOP_STATUS_BAR_HEIGHT, DESKTOP_STATUS_BAR_HEIGHT, DESKTOP_STATUS_BAR_HEIGHT) && widget_dummy.prev!=&palette_menu){
        widget_add(&start_menu);
        widget_build();
        return 1;
    }
    ret_val = widget_act(x, y);
    if(ret_val==0){
        widget_clear();
    }
    return ret_val;
}

// int GUI_widget_start(){
//     widget_add(&start_menu);
//     return 0;
// }

int GUI_widget_right_handler(int x, int y){
    if(x<IMAGE_X_DIM/2){
        right_button_menu.x = x;
    }else{
        right_button_menu.x = x-RIGHT_BUTTON_MENU_W;
    }
    right_button_menu.y = y;
    widget_clear_no_show();
    widget_add(&right_button_menu);
    widget_build();
    return 1;
}


/*
    Here are some widget examples
*/
//
// 1. Palette Menu
//
static int palette_index = 0;
unsigned char r=0, g=0, b=0;
static unsigned char palette_menu_img[PALT_MENU_X_DIM * PALT_MENU_Y_DIM];
int palette_menu_constructor(GUI_widget_t* widget){
    memset(palette_menu_img, 0x3F, PALT_MENU_X_DIM * PALT_MENU_Y_DIM);
    // draw defult color
    int i, j;
    for(i=0; i<16; i++){
        for(j=0; j<16; j++){
            draw_rect(palette_menu_img, j*8+1, i*8+1, 6, 6, PALT_MENU_X_DIM, PALT_MENU_Y_DIM, i*16+j);
        }
    }
    return 0;
}

int palette_menu_act(GUI_widget_t* widget, int x, int y){
    int index = ((y-widget->y)/8)*16 + ((x-widget->x)/8);
    if(index<0 || index>=256) return -1;
    palette_index = index;
    color_rolling_bar.x = x;
    color_rolling_bar.y = y-COLOR_ROLLING_BAR_H;
    r = g = b = 0;
    widget_add(&color_rolling_bar);
    window_list_build_screen();
    widget_build();
    return 1;
}

int palette_menu_show(GUI_widget_t* widget){
    int i, j;
    for(i=0; i<PALT_MENU_Y_DIM; i++){
        for(j=0; j<PALT_MENU_X_DIM; j++){
            GUI_screen_draw_dot(widget->x+j, widget->y+i, palette_menu_img[PALT_MENU_X_DIM*i+j]);
        }
    }
    return 0;
}


//
// 2. Color Rolling Bar
//
static unsigned char color_rolling_bar_img[COLOR_ROLLING_BAR_H*COLOR_ROLLING_BAR_W];
int color_rolling_bar_constructor(GUI_widget_t* widget){
    memset(color_rolling_bar_img, 0x3F, COLOR_ROLLING_BAR_H*COLOR_ROLLING_BAR_W);
    draw_rect(color_rolling_bar_img, 1, 9, 94, 6, COLOR_ROLLING_BAR_W, COLOR_ROLLING_BAR_H, 0x30);
    draw_rect(color_rolling_bar_img, 1, 17, 94, 6, COLOR_ROLLING_BAR_W, COLOR_ROLLING_BAR_H, 0x0C);
    draw_rect(color_rolling_bar_img, 1, 25, 94, 6, COLOR_ROLLING_BAR_W, COLOR_ROLLING_BAR_H, 0x03);
    draw_str(color_rolling_bar_img, 0, 0, "RGB Selector", COLOR_ROLLING_BAR_W, COLOR_ROLLING_BAR_H, 0);
    return 0;
}

int color_rolling_bar_act(GUI_widget_t* widget, int x, int y){
    if(is_in_range(x, y, widget->x+1, widget->y+9, 94, 6)){
        r = ((uint32_t)(x-1-widget->x))*64/94;
    }else if(is_in_range(x, y, widget->x+1, widget->y+17, 94, 6)){
        g = ((uint32_t)(x-1-widget->x))*64/94;
    }else if(is_in_range(x, y, widget->x+1, widget->y+25, 94, 6)){
        b = ((uint32_t)(x-1-widget->x))*64/94;
    }
    screen_set_one_palette(palette_index, r, g, b);
    return 0;
}

int color_rolling_bar_show(GUI_widget_t* widget){
    int i, j;
    for(i=0; i<COLOR_ROLLING_BAR_H; i++){
        for(j=0; j<COLOR_ROLLING_BAR_W; j++){
            GUI_screen_draw_dot(widget->x+j, widget->y+i, color_rolling_bar_img[COLOR_ROLLING_BAR_W*i+j]);
        }
    }
    return 0;
}



//
// 3. Start Menu
//
static unsigned char start_menu_img[START_MENU_W*START_MENU_H];
int start_menu_constructor(GUI_widget_t* widget){
    int col_x = 140;
    memset(start_menu_img, 0x3F, START_MENU_H*START_MENU_W);
    draw_str(start_menu_img, 0, 0, "Terminal Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*1, "Terminal Text Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*2, "Window Bar Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*3, "Window Bar Text Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*4, "Status Bar Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*5, "Status Bar Text Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*6, "Button 1 Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*7, "Button 2 Color:", START_MENU_W, START_MENU_H, 0);
    draw_str(start_menu_img, 0, 12*8, "Button 3 Color:", START_MENU_W, START_MENU_H, 0);
    // draw_str(start_menu_img, 0, START_MENU_H-12*4, "Sound Bar", START_MENU_W, START_MENU_H, 0x05);
    draw_str(start_menu_img, 0, START_MENU_H-12*3, "Reset to Defult Color", START_MENU_W, START_MENU_H, 0x05);
    draw_str(start_menu_img, 0, START_MENU_H-12*2, "Palette Setting", START_MENU_W, START_MENU_H, 0x05);
    draw_str(start_menu_img, 0, START_MENU_H-12, "Start Menu", START_MENU_W, START_MENU_H, 0x14);
    draw_rect(start_menu_img, col_x, 2, 8, 8, START_MENU_W, START_MENU_H, TERMINAL_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*1, 8, 8, START_MENU_W, START_MENU_H, TERMINAL_TEXT_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*2, 8, 8, START_MENU_W, START_MENU_H, WINDOW_BAR_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*3, 8, 8, START_MENU_W, START_MENU_H, WINDOW_TEXT_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*4, 8, 8, START_MENU_W, START_MENU_H, STATUS_BAR_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*5, 8, 8, START_MENU_W, START_MENU_H, STATUS_BAR_TEXT_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*6, 8, 8, START_MENU_W, START_MENU_H, BUTTON1_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*7, 8, 8, START_MENU_W, START_MENU_H, BUTTON2_COLOR);
    draw_rect(start_menu_img, col_x, 2+12*8, 8, 8, START_MENU_W, START_MENU_H, BUTTON3_COLOR);
    return 0;
}

static unsigned char item_to_color[9] = {TERMINAL_COLOR, TERMINAL_TEXT_COLOR, WINDOW_BAR_COLOR, WINDOW_TEXT_COLOR, STATUS_BAR_COLOR, STATUS_BAR_TEXT_COLOR, BUTTON1_COLOR, BUTTON2_COLOR, BUTTON3_COLOR};
int start_menu_act(GUI_widget_t* widget, int x, int y){
    uint32_t flags;
    cli_and_save(flags);
    int item_index = (y-widget->y)/12;
    if(y-widget->y>=START_MENU_H-24 && y-widget->y<=START_MENU_H-12){
        palette_menu.x = x;
        palette_menu.y = y-PALT_MENU_Y_DIM;
        widget_clear_no_show();
        widget_add(widget);
        widget_add(&palette_menu);
        window_list_build_screen();
        widget_build();
        restore_flags(flags);
        return 1;
    }else if(y-widget->y>=START_MENU_H-36 && y-widget->y<=START_MENU_H-24){
        GUI_color_reset();
        widget_clear_no_show();
        window_list_build_screen();
        widget_add(widget);
        widget_build();
        restore_flags(flags);
        return 1;
    }
    if(item_index<0 || item_index>=9 || x-widget->x<130 || y-widget->y>108){
        widget_clear_no_show();
        window_list_build_screen();
        widget_add(widget);
        widget_build();
        restore_flags(flags);
        return 1;
    }
    palette_index = item_to_color[item_index];
    color_rolling_bar.x = x;
    color_rolling_bar.y = y-COLOR_ROLLING_BAR_H;
    r = g = b = 0;
    widget_add(&color_rolling_bar);
    window_list_build_screen();
    widget_build();
    restore_flags(flags);
    return 1;
}

int start_menu_show(GUI_widget_t* widget){
    int i, j;
    for(i=0; i<START_MENU_H; i++){
        for(j=0; j<START_MENU_W; j++){
            GUI_screen_draw_dot(widget->x+j, widget->y+i, start_menu_img[START_MENU_W*i+j]);
        }
    }
    return 0;
}

//
// 4. Right Button Menu
//
static unsigned char right_button_menu_img[RIGHT_BUTTON_MENU_H*RIGHT_BUTTON_MENU_W];
int right_button_menu_constructor(GUI_widget_t* widget){
    memset(right_button_menu_img, 0x3F, RIGHT_BUTTON_MENU_H*RIGHT_BUTTON_MENU_W);
    draw_str(right_button_menu_img, 0, 0, "Full Screen", RIGHT_BUTTON_MENU_W, RIGHT_BUTTON_MENU_H, 0);
    draw_str(right_button_menu_img, 0, 12*1, "Right Half", RIGHT_BUTTON_MENU_W, RIGHT_BUTTON_MENU_H, 0);
    draw_str(right_button_menu_img, 0, 12*2, "Left Half", RIGHT_BUTTON_MENU_W, RIGHT_BUTTON_MENU_H, 0);
    draw_str(right_button_menu_img, 0, 12*3, "Minimize", RIGHT_BUTTON_MENU_W, RIGHT_BUTTON_MENU_H, 0);
    draw_str(right_button_menu_img, 0, 12*4, "Default Size", RIGHT_BUTTON_MENU_W, RIGHT_BUTTON_MENU_H, 0);
    return 0;
}

int right_button_menu_act(GUI_widget_t* widget, int x, int y){
    int index = (y-widget->y)/12;
    if(index<0 || index>=5) return -1;
    GUI_window_t* window;
    switch(index){
        case 0:
            window = window_get_top();
            if(window!=NULL){
                GUI_window_change_pos(window, 0, 0, IMAGE_X_DIM, IMAGE_Y_DIM);
            }
            break;
        case 1:
            window = window_get_top();
            if(window!=NULL){
                GUI_window_change_pos(window, IMAGE_X_DIM/2+1, 0, IMAGE_X_DIM/2, IMAGE_Y_DIM);
            }
            break;
        case 2:
            window = window_get_top();
            if(window!=NULL){
                GUI_window_change_pos(window, 0, 0, IMAGE_X_DIM/2, IMAGE_Y_DIM);
            }
            break;
        case 3:
            window = window_get_top();
            if(window!=NULL){
                status_bar_add_with_rebuild(window);
            }
            break;
        case 4:
            window = window_get_top();
            if(window!=NULL){
                GUI_window_change_pos(window, 0, 0, WINDOW_DEFULT_WIDTH, WINDOW_DEFULT_HEIGHT);
            }
            break;
        default:
            break;
    }
    widget_clear();
    return 1;
}

int right_button_menu_show(GUI_widget_t* widget){
    int i, j;
    for(i=0; i<RIGHT_BUTTON_MENU_H; i++){
        for(j=0; j<RIGHT_BUTTON_MENU_W; j++){
            GUI_screen_draw_dot(widget->x+j, widget->y+i, right_button_menu_img[RIGHT_BUTTON_MENU_W*i+j]);
        }
    }
    return 0;
}

//
// 5. Sound Bar
//
// static unsigned char sound_bar_img[SOUND_BAR_H*SOUND_BAR_W];
// int sound_bar_constructor(GUI_widget_t* widget){
//     return 0;
// }

// int sound_bar_act(GUI_widget_t* widget, int x, int y){
//     return 1;
// }

// int sound_bar_show(GUI_widget_t* widget){
//     return 0;
// }

