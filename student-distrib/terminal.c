#include "terminal.h"
#include "GUI/gui_terminal.h"
#include "text_terminal.h"
#include "process.h"
#include "scheduling.h"
#include "signal.h"

static terminal_ops_t text_terminal_ops = {
    .init = text_terminal_init,
    .putc = text_terminal_putc,
    .delc = text_terminal_delc,
    .clear = text_terminal_clear,
    .puts = text_terminal_puts,
    .printf = text_terminal_printf,
    .show = text_terminal_show,
    .exit = text_terminal_exit
};

static terminal_ops_t GUI_terminal_ops = {
    .init = GUI_terminal_init,
    .putc = GUI_terminal_putc,
    .delc = GUI_terminal_delc,
    .clear = GUI_terminal_clear,
    .puts = GUI_terminal_puts,
    .printf = GUI_terminal_printf,
    .show = GUI_terminal_show,
    .exit = GUI_terminal_exit
};

static char input_bufs[TERMINAL_NUM][TERMINAL_INPUT_BUF_SIZE + 1];
static char text_bufs[TERMINAL_NUM][TERMINAL_TEXT_BUF_SIZE];
static terminal_t terminal_arr[TERMINAL_NUM];
static int terminal_used_map[TERMINAL_NUM];

//
//  Helper Functions
//
int terminal_driver_init_each(terminal_t* terminal, terminal_ops_t* ops, char* text_buf, char* input_buf){
    terminal->start_row_index = 0;
    terminal->cur_x = 0;
    terminal->cur_y = 0;
    terminal->text_color = TERMINAL_DEFULT_TEXT_COLOR;
    terminal->background_color = TERMINAL_DEFULT_BACKGROUND_COLOR;
    terminal->ops = ops;
    terminal->text_buf = text_buf;
    terminal->input_buf_cur_pos = 0;
    terminal->input_buf = input_buf;
    terminal->occupied = NULL;
    return 0;
}



//
//  Terminal Driver Init
//
int terminal_driver_init(){
    terminal_ops_t* ops = (is_modex() ? &GUI_terminal_ops : &text_terminal_ops);
    int i;
    for(i=0; i<TERMINAL_NUM; i++){
        terminal_driver_init_each(&terminal_arr[i], ops, text_bufs[i], input_bufs[i]);
    }
    return 0;
}



//
//  Dynamic Allocation Interface
//
int terminal_index_alloc(){
    int i;
    for(i=0; i<TERMINAL_NUM; i++){
        if(terminal_used_map[i]==0){
            terminal_used_map[i] = 1;
            return i;
        }
    }
    return -1;
}

terminal_t* terminal_alloc(){
    int index = terminal_index_alloc();
    if(index==-1) return NULL;
    terminal_t* terminal = &terminal_arr[index];
    return terminal;
}

int terminal_free(terminal_t* terminal){
    int index = ((uint32_t)terminal - (uint32_t)terminal_arr) / sizeof(terminal_t);
    if(index<0 || index>=TERMINAL_NUM) return -1;
    terminal_used_map[index] = 0;
    return 0;
}

terminal_t* terminal_get(int index){
    if(index<0 || index>=TERMINAL_NUM) return NULL;
    return &terminal_arr[index];
}

int terminal_get_index(terminal_t* terminal){
    int index = ((uint32_t)terminal - (uint32_t)terminal_arr) / sizeof(terminal_t);
    if(index<0 || index>=TERMINAL_NUM) return -1;
    return index;
}

//
//  Terminal System Call Interface
//
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

int32_t terminal_close(int32_t fd){
    return 0;
}

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    //fill into the temp
    return -1;
    // terminal_t* terminal = current->terminal;
    // if (terminal == NULL){
    //     return -1;
    // }
    // if(buf==NULL || fd>FILEARR_SIZE|| fd<0 || nbytes<=0) return -1;
    // if (nbytes > TERMINAL_INPUT_BUF_SIZE){
    //     nbytes = TERMINAL_INPUT_BUF_SIZE;
    // }
    // if (terminal->input_buf_cur_pos== 0){
    //     return -1;
    // }
    // memcpy(buf, (const void*)terminal->input_buf, nbytes);
    // return nbytes;
}

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;
    terminal_t* terminal = current->terminal;
    if (terminal == NULL){
        return -1;
    }
    const char* tempbuf = buf;
    if(buf==NULL || fd>FILEARR_SIZE || fd<0 || nbytes<=0) return -1;
    cli();
    for (i=0;i<nbytes;i++){
        terminal->ops->putc(terminal, tempbuf[i]);
    }
    sti();
    return nbytes;
}



//
//  Terminal & Keyboard Interface
//
/*
when a key typed
show on the screen
input:this pointer
output:1 for success
side effect:none
*/
int terminal_keyb_handler(keyb_status_t* keyboard){
    terminal_t* terminal = top_pcb->terminal;
    // int xxx = get_pcb_index(top_process_get());
    // kprintf("terminal_index = %d, pcb_index = %d\n", terminal_get_index(terminal), get_pcb_index(top_process_get()));
    if (keyboard->bsp_flag == 1){
        bsp_handler(terminal);
        return 1;
    }
    if (keyboard->ctrl == 1){
        switch (keyboard->current_char)
        {
        case 'L':
        case 'l':
            ctrl_l_handler(terminal);
            return 1;
            break;
        case 'C':
        case 'c':
            ctrl_c_handler(terminal);
            return 1;
            break;
        default:
            break;
        }
    }
    // if (this->alt == 1){
    //     //handle alt
    // }
    switch (keyboard->current_char)
    {   
        case 0: return 1;
        // case '\n':
        //     enter_handler(terminal);
        //     break;
        default:
            if(terminal->input_buf_cur_pos < TERMINAL_INPUT_BUF_SIZE-1 && keyboard->current_char != '\n'){
                terminal_buf_put(terminal, keyboard->current_char);
                terminal->ops->putc(terminal, keyboard->current_char);
            }
            else if (terminal->input_buf_cur_pos < TERMINAL_INPUT_BUF_SIZE && keyboard->current_char == '\n'){
                terminal_buf_put(terminal, keyboard->current_char);
                terminal->ops->putc(terminal, keyboard->current_char);
            }
            break;
    }
    return 1;
}

void ctrl_l_handler(terminal_t* terminal){
    int i;
    terminal->ops->clear(terminal);
    char title[8] = "391OS> ";
    terminal->ops->puts(terminal, title);
    // write the original ones
    for (i=0;i<terminal->input_buf_cur_pos;i++){
        terminal->ops->putc(terminal, terminal->input_buf[i]);
    }
}

void ctrl_c_handler(terminal_t* terminal){
    if(top_pcb==NULL || is_PCB_legal(top_pcb)==0) return;
    kernal_send_signal(INTERRUPT, top_pcb->pid);
}

void enter_handler(terminal_t* terminal){
    terminal->ops->putc(terminal, '\n');
    terminal_buf_clear(terminal);
}

void bsp_handler(terminal_t* terminal){
    // delete text buf??????
    if (terminal->input_buf_cur_pos > 0){
        terminal_buf_delete(terminal);
        terminal->ops->delc(terminal);
    }
}

void terminal_buf_put(terminal_t* terminal, char c){
    if (terminal->input_buf_cur_pos < TERMINAL_INPUT_BUF_SIZE ){
        terminal->input_buf[terminal->input_buf_cur_pos] = c;
        terminal->input_buf_cur_pos++;
    }
    return;
}

void terminal_buf_delete(terminal_t* terminal){
    if (terminal->input_buf_cur_pos > 0){
        terminal->input_buf_cur_pos--;
        //terminal->input_buf[terminal->input_buf_cur_pos] = 0;
        return;
    }
    return;
}

void terminal_buf_clear(terminal_t* terminal){
    terminal->input_buf_cur_pos = 0;
    return;
}


//
//  Kernel Interface
//
inline terminal_t* get_current_terminal(){
    if(sched_get_pid(top_pcb)==-1) return NULL;
    return top_pcb->terminal;
    // return current->terminal;
}

int kputc(char c){
    terminal_t* terminal = get_current_terminal();
    if(terminal==NULL) return -1;
    return terminal->ops->putc(terminal, c);
}

int kdelc(){
    terminal_t* terminal = get_current_terminal();
    if(terminal==NULL) return -1;
    return terminal->ops->delc(terminal);
}

int kclear(){
    terminal_t* terminal = get_current_terminal();
    if(terminal==NULL) return -1;
    return terminal->ops->clear(terminal);
}

int kputs(char* s){
    terminal_t* terminal = get_current_terminal();
    if(terminal==NULL) return -1;
    return terminal->ops->puts(terminal, s);
}

int kprintf(char *format, ...) {
    terminal_t* terminal = get_current_terminal();
    if(terminal==NULL) return -1;
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

kprintf_format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            // screen_putc(screen, '%');
                            terminal->ops->putc(terminal, '%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto kprintf_format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    // screen_puts(screen, conv_buf);
                                    terminal->ops->puts(terminal, conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    // screen_puts(screen, &conv_buf[starting_index]);
                                    terminal->ops->puts(terminal, &conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                // screen_puts(screen, conv_buf);
                                terminal->ops->puts(terminal, conv_buf);
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
                                // screen_puts(screen, conv_buf);
                                terminal->ops->puts(terminal, conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            // screen_putc(screen, (uint8_t) *((int32_t *)esp));
                            terminal->ops->putc(terminal, (uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            // screen_puts(screen, *((int8_t **)esp));
                            terminal->ops->puts(terminal, *((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                // screen_putc(screen, *buf);
                terminal->ops->putc(terminal, *buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}


