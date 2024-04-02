#include "keyboard.h"
#include "text_terminal.h"
#include "lib.h"
#include "terminal.h"
#include "process.h"

#define VGA_MEMORY 0xB8000
#define TEXT_COLOR 0x7

char* vga_mem = (char*)VGA_MEMORY;


static int text_mem_array[3] = {VGA_TEXT_BUF_ADDR, VGA_TEXT_BUF_ADDR1,  VGA_TEXT_BUF_ADDR2};
static int video_mem_array_show[3] = {0, (VGA_TEXT_BUF_ADDR1 - VGA_TEXT_BUF_ADDR)/2,  (VGA_TEXT_BUF_ADDR2 - VGA_TEXT_BUF_ADDR)/2};


int get_addr(int index){
    if (index < 0 || index > 2){
        index = 0; // default 0
    }
    return text_mem_array[index];
}

void vga_write_set(int index){
    if (index<0 || index > 2){
        return;
        //3 terminal, range of index is from 0 to 2
    }
    vga_mem = (char*)text_mem_array[index];
    return;
}


void vga_show_set(int index){
    if (index<0 || index > 2){
        return;
        //3 terminal, range of index is from 0 to 2
    }
    uint16_t address = (uint16_t)video_mem_array_show[index];
    outb(0x0c, VGA_CMD);
    outb(((address)>>8), VGA_DATA);
    // 0x0c holds higher video memory address
    // takes higher 4 bits
    outb(0x0d, VGA_CMD);
    outb(((address)&0x00ff), VGA_DATA);
    // 0x0d holds lower video memory address
    // takes lower 4 bits
    terminal_t* terminal = terminal_get(index);
    vga_set_cursor(index, terminal->cur_x, terminal->cur_y);
    return;
}


// VGA Operations:
/*
print a char on video memory
input:x, y, c, color
output:1 for success
sideeffect: change video memory
*/
inline int vga_print(int x, int y, char c, unsigned char color){
    uint32_t index = y * TEXT_TERMINAL_COLS + x;
    *(uint8_t *)(vga_mem + (index << 1)) = c;
    *(uint8_t *)(vga_mem + (index << 1) + 1) = color;
    return 1;
}

/*
set cursor for VGA
input:x,y
output:1 for success
side effect:change vga registers
*/
inline int vga_set_cursor(int index, int x, int y){
    unsigned short pos = video_mem_array_show[index] + y * TEXT_TERMINAL_COLS + x;
    if(top_pcb==NULL || index != terminal_get_index(top_pcb->terminal)) return 0;
	outb(14, VGA_CMD); // write to 14th registers
    // 0x3d4 is command port
	outb((pos >> 8) & 0xff, VGA_DATA);
    // 0x3d5 is data port
	outb(15, VGA_CMD);  // write to 15th registers
	outb(pos & 0xff, VGA_DATA);
    return 1;
}

/*
shift VGA screen up
input:none
output:1 for success
side effect:change video memory
*/
inline int vga_up_shift(){
    unsigned int x,y;
    unsigned int index, index2;
    for(y=0;y<TEXT_TERMINAL_ROWS  -1;y++){
        for(x=0;x<TEXT_TERMINAL_COLS ;x++){
            index = y * TEXT_TERMINAL_COLS  + x;
            index2 = (y + 1) *TEXT_TERMINAL_COLS  + x;
            *(uint8_t *)(vga_mem + (index << 1)) = *(uint8_t *)(vga_mem + (index2 << 1));
            *(uint8_t *)(vga_mem + (index << 1) + 1) = *(uint8_t *)(vga_mem + (index2 << 1) + 1);
        }
    }
    for(x=0;x<TEXT_TERMINAL_COLS ;x++){
        index = y * TEXT_TERMINAL_COLS  + x;
        *(uint8_t *)(vga_mem + (index << 1)) = ' ';
    }
    return 1;
}


int text_terminal_init(terminal_t* terminal){
    // clear the screen???
    uint32_t flags;
    cli_and_save(flags);
    if (terminal == NULL){
        return -1;
    }
    int index = terminal_get_index(terminal);
    vga_write_set(index);
    int32_t i;
    for (i = 0; i < 25*80 ; i++) {  // 25 is the width, 80 is the height of screen
        *(uint8_t *)(vga_mem + (i << 1)) = ' ';
        *(uint8_t *)(vga_mem + (i << 1) + 1) = TERMINAL_DEFULT_TEXT_COLOR;
    }
    restore_flags(flags);
    terminal->start_row_index = 0;
    terminal->cur_x = 0;
    terminal->cur_y=0;
    terminal->text_color = TERMINAL_DEFULT_TEXT_COLOR;
    terminal->background_color = TERMINAL_DEFULT_BACKGROUND_COLOR;
    terminal->input_buf_cur_pos = 0;
    terminal->occupied = NULL;
    return 0;
}

int text_terminal_putc(terminal_t* terminal, char c){
    // put character into input buffer and text buffer???
    uint32_t flags;
    cli_and_save(flags);
    if (terminal == NULL){
        return -1;
    }
    int index = terminal_get_index(terminal);
    vga_write_set(index);
    if(c == '\n' || c == '\r'){
        terminal->cur_x = 0;
        if(terminal->cur_y + 1 ==  TEXT_TERMINAL_ROWS){
            vga_up_shift();
        }else{
            terminal->cur_y++;
        }
    }else{
        vga_print(terminal->cur_x, terminal->cur_y, c, terminal->text_color);
        // also record the text buf data
        terminal->text_buf[terminal->cur_x + terminal->cur_y * TEXT_TERMINAL_COLS] = c;
        // store the text at text buffer
        if(terminal->cur_x + 1 == TEXT_TERMINAL_COLS ){
            if(terminal->cur_y + 1 == TEXT_TERMINAL_ROWS){
                terminal->cur_x = 0;
                terminal->start_row_index = (terminal->start_row_index + 1) % TEXT_TERMINAL_ROWS;
                vga_up_shift();
            }else{
                terminal->cur_y++;
                terminal->cur_x = 0;
            }
        }else{
            terminal->cur_x++;
        }
    }
    vga_set_cursor(index, terminal->cur_x, terminal->cur_y);
    restore_flags(flags);
    return 1;
}

int text_terminal_delc(terminal_t* terminal){
    // delete char in the input buffer and text buffer???
    if (terminal == NULL){
        return -1;
    }
    uint32_t flags;
    cli_and_save(flags);
    int index = terminal_get_index(terminal);
    vga_write_set(index);
    if(terminal->cur_x == 0 && terminal->cur_y != 0){
        terminal->cur_y--;
        terminal->cur_x = TEXT_TERMINAL_COLS - 1;
    }else{
        terminal->cur_x--;
    }
    // screen->shift_flag = 0;
    vga_print(terminal->cur_x, terminal->cur_y, ' ', terminal->text_color);
    terminal->text_buf[terminal->cur_x + terminal->cur_y * TEXT_TERMINAL_COLS] = ' ';
    vga_set_cursor(index, terminal->cur_x, terminal->cur_y);
    restore_flags(flags);
    return 1;
}

int text_terminal_clear (terminal_t* terminal){
    terminal->start_row_index = 0;
    terminal->cur_x = 0;
    terminal->cur_y = 0;
    memset(terminal->text_buf, 0 , TEXT_TERMINAL_COLS * TEXT_TERMINAL_ROWS);
    uint32_t flags;
    cli_and_save(flags);
    if (terminal == NULL){
        return -1;
    }
    int index = terminal_get_index(terminal);
    vga_write_set(index);
    int32_t i;
    for (i = 0; i < 25*80 ; i++) {
        *(uint8_t *)(vga_mem + (i << 1)) = ' ';
        *(uint8_t *)(vga_mem + (i << 1) + 1) = 0x7;
    }
    restore_flags(flags);
    vga_set_cursor(index, terminal->cur_x, terminal->cur_y);
    return 1;
}

int text_terminal_puts (terminal_t* terminal, char* s){
    while(*s!=0){
        text_terminal_putc(terminal, *s);
        s++;
    }
    return 1;
}

int text_terminal_printf (terminal_t* terminal, char* format, ...){
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

screen_format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            text_terminal_putc(terminal, '%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto screen_format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    text_terminal_puts(terminal, conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    text_terminal_puts(terminal, &conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                text_terminal_puts(terminal, conv_buf);
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
                                text_terminal_puts(terminal, conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            text_terminal_putc(terminal, (uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            text_terminal_puts(terminal, *((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                text_terminal_putc(terminal, *buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

int text_terminal_show (terminal_t* terminal){
    // show everything in the text_buf on the screen
    if (terminal == NULL){
        return -1;
    }
    uint32_t flags;
    cli_and_save(flags);
    int index = terminal_get_index(terminal);
    vga_write_set(index);
    int i = 0;
    int x = 0;
    int y = 0;
    int num = terminal->cur_y * TEXT_TERMINAL_COLS + terminal->cur_x;
    int base = terminal->start_row_index;
    terminal->ops->clear(terminal);
    for(i=0; i<num; i++){
        vga_print(x, y, terminal->text_buf[(y+base)%TEXT_TERMINAL_ROWS+x], terminal->text_color);
        x++;
        if(x==TEXT_TERMINAL_COLS){
            x = 0;
            y++;
        }
    }
    restore_flags(flags);
    return 0;
}

int text_terminal_exit (terminal_t* terminal){
    return 0;
}


