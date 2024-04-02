
#include "process.h"
#include "keyboard.h"
#include "lib.h"
#include "interrupt.h"
#include "spinlock.h"
#include "text_terminal.h"
#include "terminal.h"
#include "scheduling.h"
#include "./GUI/window.h"
#include "./GUI/gui.h"
#include "./GUI/gui_screen.h"

// char keyboard_default_char_buffer[KEYBOARD_BUF_SIZE];
keyb_buf_t keyboard_default_buffer;
unsigned char prev_scancode = 0;
keyb_status_t keyboard;
static keyb_buf_t* keyboard_buf = &keyboard_default_buffer;


/*
initialize the keyboard
input:none
output:none
sideeffect:init keyboard
*/
void keyboard_init(){
    // set all special pressed to be 0
    //unsigned long flag;
    cli();
    keyboard.lock = SPIN_LOCK_UNLOCKED;
    keyboard.alt = 0;
    keyboard.cap_lock = 0;
    keyboard.ctrl = 0;
    keyboard.shift = 0;
    prev_scancode = 0;
    // Init defult buffer
    keyboard_buf_init(&keyboard_default_buffer);
    //keyboard_default_buffer.buf = keyboard_default_char_buffer;
    // Set buf as defult_buffer
    keyboard_buf = &keyboard_default_buffer;
    // Request keyboard irq, and enable irq
    request_irq(keyb_irq, keyboard_interrupt, 0);
    enable_irq(keyb_irq);
    //clear();
    sti();
}

void swich_terminal(unsigned char scancode){
    int index;
    switch (scancode)
    {
    case F1_CODE:
        index = 0;
        break;
    case F2_CODE:
        index = 1;
        break;
    case F3_CODE:
        index = 2;
        break;
    default:
        return;
    }
    terminal_t* terminal = terminal_get(index);
    int pid = terminal_to_sched(terminal);
    top_process_set(pid);
    if (terminal->occupied != NULL){
        window_switch(terminal->occupied);
        GUI_screen_show();
    }
    else {
        //terminal->ops->show(terminal);
        vga_show_set(index);
        vga_write_set(index);
        //terminal->ops->show(terminal);
    }
}

// reference: http://www.osdever.net/bkerndev/Docs/keyboard.htm 
// store the exact value of keyboard
// use scan code 1
char scan_code_table[STATUS_SIZE][SCANCODE_SIZE] = {
    { // none pressed， 0 position is empty
        0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   // ignore backspace and esc
        '\t',			/* Tab */
        'q', 'w', 'e', 'r',	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', //enter key 	
        0,			/* 29   - Control */
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
        '\'', '`',   0,		/* Left shift */
        '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
        'm', ',', '.', '/',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    }, 
    {   // caplock
        0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,   // ignore backspace and esc
        '\t',			/* Tab */
        'Q', 'W', 'E', 'R',	'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', //enter key 	
        0,			/* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',	/* 39 */
        '\'', '`',   0,		/* Left shift */
        '\\', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
        'M', ',', '.', '/',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    },
    { // shift pressed 
        0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   // ignore backspace and esc
        '\t',			/* Tab */
        'Q', 'W', 'E', 'R',	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', //enter key 	
        0,			/* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
        '\"', '~',   0,		/* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
        'M', '<', '>', '?',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    },
    {
        // caplock, and shift
        0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   // ignore backspace and esc
        '\t',			/* Tab */
        'Q', 'W', 'E', 'R',	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', //enter key 	
        0,			/* 29   - Control */
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
        '\"', '~',   0,		/* Left shift */
        '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
        'M', '<', '>', '?',   0,				/* Right shift */
        '*',
        0,	/* Alt */
        ' ',	/* Space bar */
        0,	/* Caps lock */
        0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
        0,	/* Scroll Lock */
        0,	/* Home key */
        0,	/* Up Arrow */
        0,	/* Page Up */
        '-',
        0,	/* Left Arrow */
        0,
        0,	/* Right Arrow */
        '+',
        0,	/* 79 - End key*/
        0,	/* Down Arrow */
        0,	/* Page Down */
        0,	/* Insert Key */
        0,	/* Delete Key */
        0,   0,   0,
        0,	/* F11 Key */
        0,	/* F12 Key */
        0,	/* All other keys are undefined */
    }
};
/*
handle scancode from keypoard and translate it into ASCII, 
return and do nothing, if nothing pressed
input:none
output:none
side effect: none
reference: http://www.osdever.net/bkerndev/Docs/keyboard.htm 
*/
int keyboard_interrupt(unsigned int ignore)
{
    unsigned char scancode;
    unsigned char scancode_lowerbit;
    unsigned char spflag = 0; // used to determine the special key being pressed
    //unsigned long flag;
    // clear();
    cli();
    keyboard_buf->current_char = 0;
    keyboard.bsp_flag = 0;
    /* Read from the keyboard's data buffer */
    scancode = inb(KEYB_DATA_PORT);
    if (scancode == 0){ //invalid scan code, clear special flags and return 
        keyboard.current_char = 0;
        keyboard.ctrl = 0;
        keyboard.shift = 0;
        keyboard.alt = 0;
        keyboard.bsp_flag = 0;
        goto keyboard_handler_finish; // no pressed return 
    }

    // printf("scancode is %x\n", scancode);
    if ((scancode & 0x80) != 0){   // the most significant bit is 1 when keyboard released 
        // key release, check for special keys, clear them
        //scancode_lowerbit &= 0x7f; // take the lower 7 bits  
        //screen_printf(&screen, "released, scancode is %x, key is %c \n", scancode, scan_code_table[0][scancode]);
        switch (scancode)
        {
        case LEFT_SHIFT_RELEASED_CODE:
            keyboard.shift = 0;  //shift
            // printf("shift released \n");
            break;
        case RIGHT_SHIFT_RELEASED_CODE:
            keyboard.shift = 0;  //shift
            // printf("shift released \n");
            break;
        case CTRL_RELEASED_CODE:
            //screen_printf(&screen, "crtl released");
            keyboard.ctrl = 0;  //ctrl
            // printf("ctrl released \n");
            break;
        case ALT_RELEASED_CODE:
            keyboard.alt = 0;  // alt
            // printf("alt released \n");
            break;
        default:
            break;
        }
        goto keyboard_handler_finish;
    } else {   // set special keys flag
        switch (scancode)
        {
        case RIGHT_SHIFT_CODE:
            keyboard.shift = 1;        // set shift flag
            break;
        case LEFT_SHIFT_CODE:
            keyboard.shift = 1;        // set shift flag
            break;
        case CAPS_CODE:
            if(prev_scancode!=scancode){
                keyboard.cap_lock ^= 1;   // switch the bit of capslock
            }
            break;
        // case ENTER_CODE:
        //     enter_handler();
        //     break;
        case BSP_CODE:
            //screen_printf(&screen, "\nbsp pressed\n");
            default_bsp_handler();
            keyboard.bsp_flag = 1;
            break;
        case CTRL_CODE:
            //screen_printf(&screen, "crtl pressed");
            keyboard.ctrl = 1;
            break;
        case ALT_CODE:
            keyboard.alt = 1; 
            break;
        // case ENTER_CODE:
        //     if (keyboard_default_buffer.cur_size == KEYBOARD_BUF_SIZE-1){
        //         screen_printf(&screen, "last enter\n");
        //         keyboard_default_buffer.buf[keyboard_default_buffer.tail] = '\n';
        //         keyboard_default_buffer.cur_size++;
        //     }
        default:
            break;
        }
    } 
    // printf("shift is %d\n", keyboard.shift);

    if (keyboard.alt == 1){
        swich_terminal(scancode);
    }
    spflag = (((keyboard.shift<<1)&0x02 ) | (keyboard.cap_lock & 0x01)); // first bit ysed for capslock, second bit used for shift
    scancode_lowerbit = scancode & 0x7f; // take lower bit, prevent overflow 
    keyboard.current_char = scan_code_table[spflag][scancode_lowerbit]; ///
    
    // if (keyboard.current_char != 0 &&(keyboard.ctrl == 0)){   // put the char when not zero
    //     if(keyboard.current_char!='\n'){
    //         keyboard_buf_put(keyboard_buf, keyboard.current_char);
    //         // terminal_char_handler(keyboard.current_char);
    //     }
    // }
    // 
    // keyboard_buf->shift = keyboard.shift;
    // keyboard_buf->cap_lock = keyboard.cap_lock;
    // keyboard_buf->ctrl = keyboard.ctrl;
    // keyboard_buf->alt = keyboard.alt;
    // keyboard_buf->current_char = keyboard.current_char;
    // keyboard_buf->bsp_flag = keyboard.bsp_flag;   // fill the information
    if (keyboard.current_char != 0 && keyboard.ctrl == 0 && keyboard.bsp_flag == 0){  // no special key and valid current char
        keyboard_buf_put(&keyboard_default_buffer, keyboard.current_char);
    }
    // keyboard_buf->handler(keyboard_buf);  // pass to terminal
    // terminal_t* terminal = current->terminal;
    //kprintf("index %d\n", terminal_get_index(terminal));
    terminal_keyb_handler(&keyboard);
keyboard_handler_finish:
    prev_scancode = scancode;
    sti();
    return 1;
}

/*
handle backspace, pop out one from the buffer
input:none
output:none
side effect:none
*/
void default_bsp_handler(){
    if (keyboard_default_buffer.cur_size > 0){
        keyboard_default_buffer.tail --;
        
        keyboard_default_buffer.cur_size --;  //decrease size and tail
        if (keyboard_default_buffer.tail < 0){  // tail is at the end
            keyboard_default_buffer.tail = KEYBOARD_BUF_SIZE-1;
        }
        return;
    }
    //screen_printf(&screen, "bsp --%d\n", keyboard_default_buffer.tail);
    return;
}


/*
put a char into current buffer
input:the char to put
output:1 for success, 0 for none
side-effect:change current keyboard char buffer
*/
int keyboard_buf_put(keyb_buf_t* buf_ptr, char c){
    //unsigned long flag;
    //screen_printf(&screen, " --%d\n", keyboard_default_buffer.tail);
    if (buf_ptr == NULL){  // check
        return 0;
    }
    if(buf_ptr->cur_size<KEYBOARD_BUF_SIZE-2 && c != '\n'){   // only enter can insert at the tail
        buf_ptr->buf[buf_ptr->tail] = c;  // put in tail
        buf_ptr->tail = (buf_ptr->tail + 1) % KEYBOARD_BUF_SIZE;  // move tail
        buf_ptr->cur_size++;
        return 1;
    }
    else if (buf_ptr->cur_size<KEYBOARD_BUF_SIZE -1 && c == '\n'){
        buf_ptr->buf[buf_ptr->tail] = c;  // put in tail
        buf_ptr->tail = (buf_ptr->tail + 1) % KEYBOARD_BUF_SIZE;  // move tail
        buf_ptr->cur_size++;
        return 1;
    }
    return 0;
}
/*
get a char from current buffer
input:none
output:the output char, 0 for fail
side-effect: change current keyboard char buffer
*/
char keyboard_buf_get(keyb_buf_t* buf_ptr){
    char c;
    if (buf_ptr == NULL){
        return 0;
    }
    if(buf_ptr->cur_size>0){ //current size greater than zero, then get a char
        c = buf_ptr->buf[buf_ptr->head];  // get from head
        buf_ptr->head = (buf_ptr->head + 1) % KEYBOARD_BUF_SIZE; // move head
        buf_ptr->cur_size--; // size--
        return c;
    }
    return 0;
}
/*
read a char from the buffer
input:buffer
output:char or 0 for fail
side effect:none
*/
char keyboard_buf_seek(keyb_buf_t* buf_ptr){
    char c;
    if (buf_ptr == NULL){
        return 0;
    }
    if(buf_ptr->cur_size>0){
        c = buf_ptr->buf[buf_ptr->head];   //seek the char at head
        return c;
    }
    return 0;
}
/*
initialize the char buffer
input:none
output:1 means success, 0 means fail
side effect: change the keyboard buffer
*/
inline int keyboard_buf_init(keyb_buf_t* buf_ptr){
    if (buf_ptr == NULL){
        return 0;
    }
    buf_ptr->cur_size = 0;
    buf_ptr->head = 0;
    buf_ptr->tail = 0;
    //buf_ptr->buf = NULL;
    buf_ptr->lock = SPIN_LOCK_UNLOCKED; //unlock
    return 1;
}


/*
set pointer points to a buffer for keyboard
input: keyboard buffer pointer
output:1 for success
side effect:change the keyboard_buf pointer
*/
int keyboard_set_buf(keyb_buf_t* buf_ptr){
    if(!buf_ptr) return 0; //invalid pointer
    keyboard_buf = buf_ptr;  // set buffer pointer
    return 1;
}


/*
set the default buffer to keyboard_buf
input:none
output:none
side effect:set buffer pointer to default one
*/
void keyboard_set_default_buffer(){
    keyboard_buf = &keyboard_default_buffer;
}

/*
get a character from buffer and won't change size
input:none
output:the char we get and -1 for fail
side-effect:change buffer
*/
char get_char(){
    if (keyboard_buf->cur_size > 0){
        return keyboard_buf_get(keyboard_buf);  // get a buffer
    }
    else {
        return 0;
    }
}

/*
read a character from the buffer
input:none
output:the buffer we read
side-effect:none
*/
char seek_char(){
    return keyboard_buf_seek(keyboard_buf);  //seek buffer
}

/*
do nothing invalid
input: fd, buf, nbytes
output:0
side effect:none
*/
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
/*
do nothing invalid
input: fd, buf, nbytes
output:0
side effect:none
*/
int32_t keyboard_open(const uint8_t *filename){
    return 0;
}
/*
do nothing invalid
input: fd, buf, nbytes
output:0
side effect:none
*/
int32_t keyboard_close(int32_t fd){
    return 0;
}


// /*
// read typed key from keyboard until enter is pressed
// input:fd, buf, nbytes
// otuput: read bytes
// sied effect: change buffer
// */
// int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes){
//     if(buf==NULL || fd>FILEARR_SIZE|| fd<0 || nbytes<=0) return -1;
//     char buf_temp[KEYBOARD_BUF_SIZE];
//     if (nbytes <= 0){
//         return -1;
//     }
//     cli();// disable interrupt
//     keyb_buf_clear();
//     sti(); //enable
//     // is the last pressed key is \n? 
//     while (keyboard_default_buffer.buf[(keyboard_default_buffer.tail-1)%KEYBOARD_BUF_SIZE] != '\n'){
//         // wait
//     }                                                  
//     // if (nbytes > (i+1)){
//     //     nbytes = i+1;
//     // }
//     // need clear interrupt
//     cli();
//     int i = 0;
//     int j = keyboard_default_buffer.head;  
//     while(keyboard_default_buffer.buf[j] != '\n'){ // read until \n
//         buf_temp[i] = keyboard_default_buffer.buf[j];
//         i++;
//         j = (j+1)%KEYBOARD_BUF_SIZE;
//         //screen_printf(&screen, "in while\n");
//     }
//     sti();
//     //screen_printf(&screen, "i is %d, copy %d bytes\n", i, i+1);
//     buf_temp[i] = '\n';
//     memcpy(buf, (const void*)buf_temp, (i+1));
//     // bbbb = (char*)buf;
//     // int k = 0;
//     // while (bbbb[k] != '\n'){
//     //     screen_printf(&screen, "buffer %dth is %c\n", k, bbbb[k]);
//     //     k++;
//     // }
//     // screen_printf(&screen, "                get out of while\n");
//     return i + 1; //i+1 copied
// }

/*
read typed key from keyboard until enter is pressed
input:fd, buf, nbytes
otuput: read bytes
sied effect: change buffer
*/
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes){
    if(buf==NULL || fd>FILEARR_SIZE|| fd<0 || nbytes<=0) return -1;
    if (nbytes <= 0){
        return -1;
    }
    cli();// disable interrupt
    terminal_t* terminal = current->terminal;
    terminal_buf_clear(terminal);
    sti(); //enable
    // is the last pressed key is \n? 
    while (terminal->input_buf_cur_pos == 0 || terminal->input_buf[terminal->input_buf_cur_pos-1] != '\n'){
        // wait
    }                                                  
    // if (nbytes > (i+1)){
    //     nbytes = i+1;
    // }
    // need clear interrupt
    cli();
    int i = terminal->input_buf_cur_pos;
    // int j;
    // for (j=0;j<terminal->input_buf_cur_pos;j++){
    //     if (terminal->input_buf[j] == '\n'){
    //         kprintf("enter");
    //         break;
    //     }
    //     kprintf(" %c ", terminal->input_buf[j]);
    // }
    // kprintf("\n%d\n", terminal->input_buf_cur_pos);
    sti();
    //screen_printf(&screen, "i is %d, copy %d bytes\n", i, i+1);
    memcpy(buf, (const void*)terminal->input_buf, (i));
    // bbbb = (char*)buf;
    // int k = 0;
    // while (bbbb[k] != '\n'){
    //     screen_printf(&screen, "buffer %dth is %c\n", k, bbbb[k]);
    //     k++;
    // }
    // screen_printf(&screen, "                get out of while\n");
    return i; //i copied
}




/*
claer keyboard
input:none
output:none
side effect:clear the whole keyboard buffer
*/
void keyb_buf_clear(){
    int i;
    keyboard_default_buffer.head = 0;
    keyboard_default_buffer.tail = 0;
    keyboard_default_buffer.cur_size = 0; //set everything to be 0
    for (i=0;i<KEYBOARD_BUF_SIZE;i++){
        keyboard_default_buffer.buf[i] = 0;  //clear character last time
    }
}
