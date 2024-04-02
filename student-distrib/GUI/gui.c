#include "gui.h"
#include "../lib.h"
#include "window.h"
#include "../syscall.h"
#include "mouse.h"
#include "../scheduling.h"
#include "gui_screen.h"
#include "gui_interaction.h"
#include "gui_color.h"

int32_t window_list_init();

//
// GUI init
//
int GUI_init(){
    GUI_build_background(NULL);
    GUI_desktop_init();
    GUI_screen_init();
    GUI_color_init();
    window_list_init();
    widget_driver_init();
    return 0;
}



//
// GUI window relation handlers
//
typedef struct window_list window_list_t;
struct window_list{
    window_list_t* next;
    window_list_t* prev;
    int32_t window_index;   // can not be changed after init
    GUI_window_t* window;
};
static window_list_t window_list_dummy = {&window_list_dummy, &window_list_dummy, -1};
static window_list_t window_list_arr[WINDOW_MAX_NUM];
static unsigned char rel_screen_buf[IMAGE_X_DIM * IMAGE_Y_DIM];     // plane 3, 2, 1, 0
static char rel_screen_occupied[IMAGE_X_DIM * IMAGE_Y_DIM];    // (x, y)
static char window_inuse[WINDOW_MAX_NUM];

int32_t window_list_init(){
    window_list_dummy.next = &window_list_dummy;
    window_list_dummy.prev = &window_list_dummy;
    window_list_dummy.window_index = -1;
    int i;
    for(i=0; i<WINDOW_MAX_NUM; i++){
        window_list_arr[i].next = NULL;
        window_list_arr[i].prev = NULL;
        window_list_arr[i].window_index = i;
        window_list_arr[i].window = GUI_window_get(i);
    }
    return 0;
}

int32_t window_list_add_head(GUI_window_t* window){
    int32_t index = GUI_window_get_index(window);
    if(index==-1) return -1;
    uint32_t flags;
    cli_and_save(flags);
    window_list_arr[index].prev = &window_list_dummy;
    window_list_arr[index].next = window_list_dummy.next;
    window_list_arr[index].prev->next = &window_list_arr[index];
    window_list_arr[index].next->prev = &window_list_arr[index];
    restore_flags(flags);
    return 0;
}

int32_t window_list_add_tail(GUI_window_t* window){
    int32_t index = GUI_window_get_index(window);
    if(index==-1) return -1;
    uint32_t flags;
    cli_and_save(flags);
    window_list_arr[index].prev = window_list_dummy.prev;
    window_list_arr[index].next = &window_list_dummy;
    window_list_arr[index].prev->next = &window_list_arr[index];
    window_list_arr[index].next->prev = &window_list_arr[index];
    restore_flags(flags);
    return 0;
}

int32_t window_list_delete(GUI_window_t* window){
    int32_t index = GUI_window_get_index(window);
    if(index==-1 || window_list_arr[index].prev==NULL || window_list_arr[index].next==NULL) return -1;
    uint32_t flags;
    cli_and_save(flags);
    window_list_arr[index].prev->next = window_list_arr[index].next;
    window_list_arr[index].next->prev = window_list_arr[index].prev;
    window_list_arr[index].prev = NULL;
    window_list_arr[index].next = NULL;
    restore_flags(flags);
    return 0;
}

int32_t window_list_move_to_top(GUI_window_t* window){
    int32_t index = GUI_window_get_index(window);
    if(index==-1 || window_list_arr[index].prev==NULL || window_list_arr[index].next==NULL) return -1;
    uint32_t flags;
    cli_and_save(flags);
    window_list_delete(window);
    window_list_add_head(window);
    restore_flags(flags);
    return 0;
}

int32_t window_list_move_to_bottom(GUI_window_t* window){
    int32_t index = GUI_window_get_index(window);
    if(index==-1 || window_list_arr[index].prev==NULL || window_list_arr[index].next==NULL) return -1;
    uint32_t flags;
    cli_and_save(flags);
    window_list_delete(window);
    window_list_add_tail(window);
    restore_flags(flags);
    return 0;
}

int32_t window_list_build_screen(){
    window_list_t* ptr;
    uint32_t flags;
    cli_and_save(flags);
    GUI_screen_clear_without_show();
    memcpy(get_desktop_img(), get_background_img(), IMAGE_X_DIM * IMAGE_Y_DIM);
    memcpy(cur_screen_buf, get_background_img(), IMAGE_X_DIM * IMAGE_Y_DIM);
    memset(rel_screen_occupied, -1, IMAGE_X_DIM * IMAGE_Y_DIM);
    memset(window_inuse, 0, WINDOW_MAX_NUM * sizeof(*window_inuse));
    for(ptr = window_list_dummy.prev; ptr != window_list_dummy.next; ptr = ptr->prev){
        GUI_window_show(GUI_window_get(ptr->window_index));
        window_inuse[ptr->window_index] = 1;
        int i, j;
        for(i=ptr->window->Wy; i<ptr->window->Wy+ptr->window->Wh; i++){
            for(j=ptr->window->Wx; j<ptr->window->Wx+ptr->window->Ww; j++){
                if(i>=0 && i<IMAGE_Y_DIM && j>=0 && j<IMAGE_X_DIM){
                    rel_screen_occupied[i*IMAGE_X_DIM+j] = ptr->window_index;
                }
            }
        }
    }
    memcpy(rel_screen_buf, cur_screen_buf, IMAGE_X_DIM * IMAGE_Y_DIM);
    if(window_list_dummy.next != &window_list_dummy){
        GUI_window_show(GUI_window_get(window_list_dummy.next->window_index));
    }
    memcpy(get_desktop_img(), rel_screen_buf, IMAGE_X_DIM * IMAGE_Y_DIM);
    restore_flags(flags);
    return 0;
}

int32_t window_list_is_inuse(GUI_window_t* window){
    if(window==NULL) return -1;
    return window_inuse[GUI_window_get_index(window)];
}

int32_t is_screen_no_window(){
    return window_list_dummy.next == &window_list_dummy;
}

// static unsigned char tmpbuf[IMAGE_X_DIM * IMAGE_Y_DIM];
int32_t window_switch(GUI_window_t* window){
    uint32_t flags;
    if(window==NULL) return -1;
    if(window->is_show==0) return -1;
    cli_and_save(flags);
    window_list_delete(window);
    window_list_add_head(window);
    window_list_build_screen();
    restore_flags(flags);
    // screen_display(rel_screen_buf);
    // screen_display(rel_screen_occupied);
    // int i, j;
    // for(i=0; i<IMAGE_Y_DIM; i++){
    //     for(j=0; j<IMAGE_X_DIM; j++){
    //         if(rel_screen_buf[i*IMAGE_X_DIM+j]==-1) tmpbuf[i*IMAGE_X_DIM+j] = 0x3F;
    //         if(rel_screen_buf[i*IMAGE_X_DIM+j]==0)  tmpbuf[i*IMAGE_X_DIM+j] = 0x30;
    //         if(rel_screen_buf[i*IMAGE_X_DIM+j]==1)  tmpbuf[i*IMAGE_X_DIM+j] = 0x0F;
    //     }
    // }
    return 0;
}

int32_t window_remove(GUI_window_t* window){
    uint32_t flags;
    if(window==NULL) return -1;
    cli_and_save(flags);
    window_list_delete(window);
    window_list_build_screen();
    restore_flags(flags);
    return 0;
}

GUI_window_t* window_get_top(){
    if(window_list_dummy.next == &window_list_dummy) return NULL;
    return window_list_dummy.next->window;
}

GUI_window_t* get_window_from_screen_coordinate(int x, int y){
    if(x<0 || x>=IMAGE_X_DIM || y<0 || y>=IMAGE_Y_DIM) return NULL;
    GUI_window_t* top = window_get_top();
    if(top==NULL) return NULL;
    if(x>=top->Wx && x<top->Wx+top->Ww && y>=top->Wy && y<top->Wy+top->Wh){
        return top;
    }
    if(rel_screen_occupied[y*IMAGE_X_DIM+x]==-1) return NULL;
    return window_list_arr[(uint32_t)rel_screen_occupied[y*IMAGE_X_DIM+x]].window;
}



//
// Status Bar
//
static GUI_window_t* status_bar_window[DESKTOP_STATUS_BAR_NUM];
int status_bar_draw_block(int block, char* str){
    if(str==NULL || block<0 || block>=DESKTOP_STATUS_BAR_NUM) return -1;
    int x = STATUS_BAR_BLOCK_START_X + block * STATUS_BAR_BLOCK_WIDTH;
    while(*str!=0){
        GUI_status_bar_draw_font_defult_size(x, *str, DESKTOP_STATUS_BAR_SPLIT_COLOR);
        x += FONT_WIDTH;
        str++;
    }
    return 0;
}

int status_bar_clear_block(int block){
    if(block<0 || block>=DESKTOP_STATUS_BAR_NUM) return -1;
    int i, j;
    int start_x = STATUS_BAR_BLOCK_START_X + block*STATUS_BAR_BLOCK_WIDTH;
    for(i=0; i<DESKTOP_STATUS_BAR_HEIGHT; i++){
        for(j=0; j<STATUS_BAR_BLOCK_WIDTH; j++){
            GUI_status_bar_draw_dot(start_x+j, i, DESKTOP_STATUS_BAR_COLOR);
        }
    }
    return 0;
}

int status_bar_add(GUI_window_t* window){
    int i;
    char name[6];
    if(window==NULL) return -1;
    for(i=0; i<DESKTOP_STATUS_BAR_NUM; i++){
        if(status_bar_window[i]==NULL){
            status_bar_window[i] = window;
            window->is_show = 0;
            int len = strlen(window->name);
            if(len<=4){
                status_bar_draw_block(i, window->name);
            }else{
                strncpy(name, window->name, 5);
                name[5] = 0;
                status_bar_draw_block(i, name);
            }
            window_list_delete(window);
            return i;
        }
    }
    return -1;
}

int status_bar_del(GUI_window_t* window){
    int i;
    if(window==NULL) return -1;
    for(i=0; i<DESKTOP_STATUS_BAR_NUM; i++){
        if(status_bar_window[i]==window){
            status_bar_window[i] = NULL;
            window->is_show = 1;
            status_bar_clear_block(i);
            window_list_add_head(window);
            return i;
        }
    }
    return -1;
}

GUI_window_t* status_bar_get(int index){
    if(index<0 || index>=DESKTOP_STATUS_BAR_NUM) return NULL;
    return status_bar_window[index];
}

GUI_window_t* status_bar_get_from_x(int x){
    if(x<0 || x>=IMAGE_Y_DIM || (x/STATUS_BAR_BLOCK_WIDTH>=DESKTOP_STATUS_BAR_NUM) || x<STATUS_BAR_BLOCK_START_X) return NULL;
    return status_bar_window[(x-STATUS_BAR_BLOCK_START_X)/STATUS_BAR_BLOCK_WIDTH];
}

int status_bar_del_from_x(int x){
    GUI_window_t* window = status_bar_get_from_x(x);
    if(window==NULL) return -1;
    return status_bar_del(window);
}

int update_top_process_by_window(){
    if(window_get_top()==NULL) return -1;
    int32_t to_pid = sched_get_pid(get_pcb_from_pos(window_get_top()->owner_pid));
    if(to_pid==-1) return -1;
    top_process_set(to_pid);
    return 0;
}

void status_bar_add_with_rebuild(GUI_window_t* window){
    if(window==NULL) return;
    status_bar_add(window);
    window_list_build_screen();
    update_top_process_by_window();
}

void status_bar_del_with_rebuild(int x){
    status_bar_del_from_x(x);
    window_list_build_screen();
    update_top_process_by_window();
}



//
// GUI mouse handlers
//
static int mouse_old_left = 0;
static int mouse_old_x = 0;
static int mouse_old_y = 0;
// static int mouse_full_screen = 0;
static GUI_window_t* mouse_hold_window = NULL;
static enum mouse_op_t{
    NONE,
    RIGHT_EDGE_MOVE,
    LEFT_EDGE_MOVE,
    LOWER_EDGE_MOVE,
    WINDOW_MOVE
}mouse_op;

volatile static unsigned char mouse_color = GUI_MOUSE_DEFULT_COLOR;

unsigned char get_mouse_color(){
    return mouse_color;
}

int is_in_range(int x, int y, int xs, int ys, int w, int h){
    if(x>=xs && x<xs+w && y>=ys && y<ys+h) return 1;
    else return 0;
}

int GUI_mouse_handler(mouse_t mouse){
    GUI_window_t* window;
    if(is_PCB_legal(current) == 0 || sched_safety_check()==0) return -1;
    window = window_get_top();
    int x = mouse.x;
    int y = mouse.y;
    int hold = mouse.left;
    int right = mouse.right;
    if(hold==1){
        if(GUI_widget_handler(x, y)==1){
            mouse_hold_window = NULL;
            mouse_op = NONE;
            goto GUI_mouse_show_screen;
        }
    }
    if(window==NULL){
        mouse_old_left=0;
        if(hold==1){
            if(get_window_from_screen_coordinate(x, y)==NULL && is_in_range(x, y, 0, IMAGE_Y_DIM-DESKTOP_STATUS_BAR_HEIGHT, IMAGE_X_DIM, DESKTOP_STATUS_BAR_HEIGHT)){
                window = status_bar_get_from_x(x);
                if(window!=NULL){
                    status_bar_del_with_rebuild(x);
                }
            }
        }
        goto GUI_mouse_show_screen;
    }
    if(right==1){
        if(is_in_range(x, y, window->Wx, window->Wy, window->Ww, window->Py-window->Wy)){
            GUI_widget_right_handler(x, y);
        }
    }
    int button_x = get_button_start_x(window);
    int button_y = get_button_start_y(window);
    int diff_x, diff_y;
    if(hold==1){
        if(mouse_old_left==0){
            mouse_old_x = x;
            mouse_old_y = y;
            mouse_old_left = 1;
            if(is_in_range(x, y, button_x+WINDOW_BAR_BUTTON_WIDTH, button_y, WINDOW_BAR_BUTTON_WIDTH, WINDOW_BAR_BUTTON_HEIGHT)){
                if(window->Wx==0 && window->Wy==0 && window->Ww==IMAGE_X_DIM && window->Wh==IMAGE_Y_DIM){
                    GUI_window_change_pos(window, 0, 0, WINDOW_DEFULT_WIDTH, WINDOW_DEFULT_HEIGHT);
                }else{
                    GUI_window_change_pos(window, 0, 0, IMAGE_X_DIM, IMAGE_Y_DIM);
                }
                goto GUI_mouse_show_screen;
            }else if(is_in_range(x, y, button_x+2*WINDOW_BAR_BUTTON_WIDTH, button_y, WINDOW_BAR_BUTTON_WIDTH, WINDOW_BAR_BUTTON_HEIGHT)){
                kernal_send_signal(INTERRUPT, top_pcb->pid);
                goto GUI_mouse_show_screen;
            }else if(is_in_range(x, y, button_x, button_y, WINDOW_BAR_BUTTON_WIDTH, WINDOW_BAR_BUTTON_HEIGHT)){
                // 任务栏最小化
                status_bar_add_with_rebuild(window);
                goto GUI_mouse_show_screen;
            }
            // check move and size changes
            if(x>=window->Wx && x<window->Wx+window->Ww && y>=window->Wy && y<window->Py){
                mouse_op = WINDOW_MOVE;
                mouse_hold_window = window;
            }else if(x>=window->Wx && x<window->Px && y>=window->Py && y<window->Wy+window->Wh){     // left edge
                mouse_op = LEFT_EDGE_MOVE;
                mouse_hold_window = window;
            }else if(x>=window->Px+window->Pw && x<window->Wx+window->Ww && y>=window->Py && y<window->Wy+window->Wh){   // right dege
                mouse_op = RIGHT_EDGE_MOVE;
                mouse_hold_window = window;
            }else if(x>=window->Wx && x<window->Wx+window->Ww && y>=window->Py+window->Ph && y<window->Wy+window->Wh){   // lower edge
                mouse_op = LOWER_EDGE_MOVE;
                mouse_hold_window = window;
            }else if(x>=window->Wx && x<window->Wx+window->Ww && y>=window->Wy && y<window->Wy+window->Wh){ 
                mouse_op = NONE;
                mouse_hold_window = window;
            }else{
                mouse_op = NONE;
                mouse_hold_window = get_window_from_screen_coordinate(x, y);
                if(mouse_hold_window==NULL){
                    if(get_window_from_screen_coordinate(x, y)==NULL && is_in_range(x, y, 0, IMAGE_Y_DIM-DESKTOP_STATUS_BAR_HEIGHT, IMAGE_X_DIM, DESKTOP_STATUS_BAR_HEIGHT)){
                        window = status_bar_get_from_x(x);
                        if(window!=NULL){
                            status_bar_del_with_rebuild(x);
                        }
                    }
                }
            }
        }
        mouse_color = GUI_MOUSE_HOLD_COLOR;
    }else{
        switch(mouse_op){
            case WINDOW_MOVE:
                diff_x = x - mouse_old_x;
                diff_y = y - mouse_old_y;
                GUI_window_change_pos(window, window->Wx+diff_x, window->Wy+diff_y, window->Ww, window->Wh);
                mouse_color = GUI_MOUSE_DEFULT_COLOR;
                break;
            case LEFT_EDGE_MOVE:
                diff_x = x - mouse_old_x;
                GUI_window_change_pos(window, window->Wx+diff_x, window->Wy, window->Ww-diff_x, window->Wh);
                mouse_color = GUI_MOUSE_DEFULT_COLOR;
                break;
            case RIGHT_EDGE_MOVE:
                diff_x = x - mouse_old_x;
                GUI_window_change_pos(window, window->Wx, window->Wy, window->Ww+diff_x, window->Wh);
                mouse_color = GUI_MOUSE_DEFULT_COLOR;
                break;
            case LOWER_EDGE_MOVE:
                diff_y = y - mouse_old_y;
                GUI_window_change_pos(window, window->Wx, window->Wy, window->Ww, window->Wh+diff_y);
                mouse_color = GUI_MOUSE_DEFULT_COLOR;
                break;
            default:
                if(x>=window->Wx && x<window->Wx+window->Ww && y>=window->Wy && y<window->Py){
                    mouse_color = GUI_MOUSE_EDGE_COLOR;
                }else if(x>=window->Wx && x<window->Px && y>=window->Py && y<window->Wy+window->Wh){     // left edge
                    mouse_color = GUI_MOUSE_EDGE_COLOR;
                }else if(x>=window->Px+window->Pw && x<window->Wx+window->Ww && y>=window->Py && y<window->Wy+window->Wh){   // right dege
                    mouse_color = GUI_MOUSE_EDGE_COLOR;
                }else if(x>=window->Wx && x<window->Wx+window->Ww && y>=window->Py+window->Ph && y<window->Wy+window->Wh){   // lower edge
                    mouse_color = GUI_MOUSE_EDGE_COLOR;
                }else{ 
                    mouse_color = GUI_MOUSE_DEFULT_COLOR;
                }
                if(mouse_hold_window!=NULL && mouse_hold_window!=window_get_top() && window_list_is_inuse(mouse_hold_window)==1){
                    window_switch(mouse_hold_window);
                    update_top_process_by_window();
                }
                break;
        }
        mouse_old_left = 0;
        mouse_op = NONE;
        mouse_hold_window = NULL;
    }

GUI_mouse_show_screen:
    GUI_screen_show();

// TEST CODE:
    // draw_palette_menu();
    // show_palette_menu(0, 0);

    return 0;
}


