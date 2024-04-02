#include "gui_terminal.h"
#include "../process.h"
#include "screen.h"
#include "gui_screen.h"

//
// Helper Functions
//
inline int _GUI_terminal_print(terminal_t* terminal, char c){
    int x = terminal->cur_x * GUI_TERMINAL_CHAR_W;
    int y = terminal->cur_y * GUI_TERMINAL_CHAR_H;
    terminal->text_buf[((terminal->start_row_index + terminal->cur_y) % GUI_TERMINAL_NUM_Y) * GUI_TERMINAL_NUM_X + terminal->cur_x] = c;
    GUI_window_draw_font(terminal->occupied, x, y, GUI_TERMINAL_CHAR_W, GUI_TERMINAL_CHAR_H, c, terminal->text_color);
    return 0;
}

inline int _GUI_dir_terminal_printc(terminal_t* terminal, int nx, int ny, char c){
    int x = nx * GUI_TERMINAL_CHAR_W;
    int y = ny * GUI_TERMINAL_CHAR_H;
    GUI_window_draw_font(terminal->occupied, x, y, GUI_TERMINAL_CHAR_W, GUI_TERMINAL_CHAR_H, c, terminal->text_color);
    return 0;
}

inline int _GUI_terminal_up_shift(terminal_t* terminal){
    int i, j;
    int lastline = ((terminal->cur_y + terminal->start_row_index)%GUI_TERMINAL_NUM_Y)*GUI_TERMINAL_NUM_X;
    for(i=0; i<GUI_TERMINAL_NUM_X; i++) terminal->text_buf[lastline + i] = ' ';
    GUI_window_t* window = terminal->occupied;
    GUI_window_clear(window, terminal->background_color);
    for(i=0; i<GUI_TERMINAL_NUM_Y; i++){
        lastline = ((terminal->start_row_index + i) % GUI_TERMINAL_NUM_Y) * GUI_TERMINAL_NUM_X;
        for(j=0; j<GUI_TERMINAL_NUM_X; j++){
            int x = j * GUI_TERMINAL_CHAR_W;
            int y = i * GUI_TERMINAL_CHAR_H;
            char c = terminal->text_buf[lastline + j];
            GUI_window_draw_font(window, x, y, GUI_TERMINAL_CHAR_W, GUI_TERMINAL_CHAR_H, c, terminal->text_color);
        }
    }
    return 0;
}

inline int _GUI_terminal_putc(terminal_t* terminal, char c){
    if(c == '\n' || c == '\r'){
        terminal->cur_x = 0;
        if(terminal->cur_y + 1 == GUI_TERMINAL_NUM_Y){
            terminal->start_row_index = (terminal->start_row_index + 1) % GUI_TERMINAL_NUM_Y;
            _GUI_terminal_up_shift(terminal);
        }else{
            terminal->cur_y++;
        }
    }else{
        _GUI_terminal_print(terminal, c);
        if(terminal->cur_x + 1 == GUI_TERMINAL_NUM_X){
            if(terminal->cur_y + 1 == GUI_TERMINAL_NUM_Y){
                terminal->cur_x = 0;
                terminal->start_row_index = (terminal->start_row_index + 1) % GUI_TERMINAL_NUM_Y;
                _GUI_terminal_up_shift(terminal);
            }else{
                terminal->cur_y++;
                terminal->cur_x = 0;
            }
        }else{
            terminal->cur_x++;
        }
    }
    return 0;
}

int _GUI_terminal_delc(terminal_t* terminal){
    if(terminal==NULL) return -1;
    if(terminal->cur_x == 0 && terminal->cur_y != 0){
        terminal->cur_y--;
        terminal->cur_x = GUI_TERMINAL_NUM_X - 1;
    }else{
        terminal->cur_x--;
    }
    int x = terminal->cur_x * GUI_TERMINAL_CHAR_W;
    int y = terminal->cur_y * GUI_TERMINAL_CHAR_H;
    terminal->text_buf[((terminal->start_row_index + terminal->cur_y) % GUI_TERMINAL_NUM_Y) * GUI_TERMINAL_NUM_X + terminal->cur_x] = 0;
    GUI_window_draw_font_over(terminal->occupied, x, y, GUI_TERMINAL_CHAR_W, GUI_TERMINAL_CHAR_H, ' ', terminal->text_color, terminal->background_color);
    return 0;
}

int _GUI_terminal_puts(terminal_t* terminal, char* s){
    if(terminal==NULL || s==NULL) return -1;
    while(*s!=0){
        GUI_terminal_putc(terminal, *s);
        s++;
    }
    return 0;
}

//
// GUI Terminal Interfaces
//
int GUI_terminal_init (terminal_t* terminal){
    terminal->start_row_index = 0;
    terminal->cur_x = 0;
    terminal->cur_y = 0;
    terminal->text_color = GUI_TERMINAL_DEFULT_CHAR_COLOR;
    terminal->background_color = GUI_TERMINAL_DEFULT_BACKGROUND_COLOR;
    terminal->input_buf_cur_pos = 0;
    memset(terminal->text_buf, 0, GUI_TERMINAL_NUM_X * GUI_TERMINAL_NUM_Y);
    memset(terminal->input_buf, 0, TERMINAL_INPUT_BUF_SIZE);
    GUI_window_clear(terminal->occupied, terminal->background_color);
    GUI_window_show(terminal->occupied);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_putc (terminal_t* terminal, char c){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    _GUI_terminal_putc(terminal, c);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_delc (terminal_t* terminal){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    _GUI_terminal_delc(terminal);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_clear (terminal_t* terminal){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    terminal->start_row_index = 0;
    terminal->cur_x = 0;
    terminal->cur_y = 0;
    memset(terminal->text_buf, 0, GUI_TERMINAL_NUM_X * GUI_TERMINAL_NUM_Y);
    GUI_window_clear(terminal->occupied, terminal->background_color);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_puts (terminal_t* terminal, char* s){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    _GUI_terminal_puts(terminal, s);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_show (terminal_t* terminal){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    int base = terminal->start_row_index;
    int num = terminal->cur_y * GUI_TERMINAL_NUM_X + terminal->cur_x;
    int x = 0;
    int y = 0;
    int i = 0;
    GUI_window_clear(terminal->occupied, terminal->background_color);
    for(i=0; i<num; i++){
        _GUI_dir_terminal_printc(terminal, x, y, terminal->text_buf[((y+base)%GUI_TERMINAL_NUM_Y)*GUI_TERMINAL_NUM_X+x]);
        x++;
        if(x==GUI_TERMINAL_NUM_X){
            x = 0;
            y++;
        }
    }
    GUI_window_show(terminal->occupied);
    GUI_screen_show();
    return 0;
}

int GUI_terminal_exit (terminal_t* terminal){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    GUI_window_free(terminal->occupied);
    return 0;
}

int GUI_terminal_printf (terminal_t* terminal, char* format, ...){
    if(terminal==NULL || terminal->occupied==NULL) return -1;
    /* Pointer to the gfgfformat string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

GUI_terminal_format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            _GUI_terminal_putc(terminal, '%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto GUI_terminal_format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    _GUI_terminal_puts(terminal, conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    _GUI_terminal_puts(terminal, &conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                _GUI_terminal_puts(terminal, conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                _GUI_terminal_puts(terminal, conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            _GUI_terminal_putc(terminal, (uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            _GUI_terminal_puts(terminal, *((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                _GUI_terminal_putc(terminal, *buf);
                break;
        }
        buf++;
    }
    // GUI_window_show((GUI_window_t*)terminal);
    GUI_screen_show();
    return (buf - format);
}


