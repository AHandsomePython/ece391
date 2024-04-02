#include "mouse.h"
#include "../interrupt.h"
#include "../terminal.h"
#include "screen.h"
#include "gui.h"

// Referance: https://wiki.osdev.org/Mouse_Input

volatile mouse_t mouse;
static uint8_t status;
volatile int read_flag;

void mouse_init(){
    cli();
    mouse_aux_enable();
    mouse_reset();
    mouse_set_compaq_byte();
    mouse_change_sample(40);
    mouse_change_resol(2);
    mouse_enable_streaming();
    request_irq(12, mouse_handler, 0);
    enable_irq(12);
    //screen_printf(&screen, "success mouse\n");
    sti();
    return;
}

mouse_t mouse_cur_buf;
mouse_t* get_mouse(){
    mouse_cur_buf = mouse;
    return &mouse_cur_buf;
}

static int old_left = 0;

int mouse_handler(unsigned int irq){
    if ( !(inb(MOUSE_STATUS) & 0x1)) {
        read_flag = 0;
        return 0;
    } 
    else {
        read_flag = 1;
        mouse.byte1 = inb(MOUSE_DATA);
    }
    if (mouse.byte1 == MOUSE_ACK) {
        return 0;
    }

    //mouse_wait_before_read();
    
    // if(mouse.byte1[6] || mouse.byte1[7] || !mouse.byte1[3]) {
    //     send_eoi(MOUSE_IRQ);
    //     sti();
    //     return;
    // }
    
    int new_left = mouse.byte1 & 0x1;
    if(new_left==old_left){
        mouse.left = new_left;
    }
    old_left = new_left;
    // mouse.left = mouse.byte1 & 0x1;
    mouse.right = (mouse.byte1 & 0x2)>>1; 
    mouse_wait_before_read();
    mouse.x_change = inb(MOUSE_DATA);
    mouse.x += mouse.x_change / MOUSE_DEC_RATE_X;
    if(mouse.x>=IMAGE_X_DIM) mouse.x = IMAGE_X_DIM-1;
    if(mouse.x<0) mouse.x = 0;
    mouse_wait_before_read();
    mouse.y_change = inb(MOUSE_DATA);
    mouse.y -= mouse.y_change / MOUSE_DEC_RATE_Y;
    if(mouse.y>=IMAGE_Y_DIM) mouse.y = IMAGE_Y_DIM-1;
    if(mouse.y<0) mouse.y = 0;

    // if(mouse.right == 1) screen_printf(&screen,"right is pressed");
    // if(mouse.left == 1) screen_printf(&screen,"left is pressed");
    // screen_printf(&screen,"x: %d y: %d\n",mouse.x,mouse.y);

    GUI_mouse_handler(mouse);

    return 1;
}

void mouse_write_(uint8_t port, uint8_t data){
    mouse_wait_before_write();
    outb(data,port);
}


uint8_t mouse_read_(uint8_t port){
    mouse_wait_before_read();
    read_flag = 1;
    return inb(port);
}




// check if 0th bit of 0x64 is set to 1, then can read from mouse
// return value 1 means available to read.
int mouse_wait_before_read(){
    while(!(inb(MOUSE_STATUS) & 0x1)){
    }
    return 1;
}

// check if 1th bit of 0x64 is cleared, then can output to mouse
// return value 1 means available to write.
int mouse_wait_before_write(){
    while(inb(MOUSE_STATUS) & 0x2){

    }
    return 1;
}

void mouse_aux_enable(){
    if(mouse_wait_before_write()){
        outb(MOUSE_ENABLE_AUX,MOUSE_STATUS);
    }
}


void mouse_set_compaq_byte(){
    if(mouse_wait_before_write()){
        outb(GET_COMPAQ_STA,MOUSE_STATUS);
    }

    if(mouse_wait_before_read()){
        status = inb(MOUSE_DATA); // get status bits
    }
    status |= 0x2; // need to set bit number 1 to Enable IRQ12
    status &= 0xDF;// and clear bit number 5 to Disable Mouse Clock
    
    if(mouse_wait_before_write()){
        outb(MOUSE_DATA,MOUSE_STATUS);
    }

    if(mouse_wait_before_write()){
        outb(status,MOUSE_DATA);
    }
}

void mouse_enable_streaming(){
    mouse_send_cmd();
    if(mouse_wait_before_write()){
        outb(ENABLE_STREAM,MOUSE_DATA);
    }
    mouse_wait_ack();
}

void mouse_send_cmd(){
    if(mouse_wait_before_write()){
        outb(MOUSE_CMD,MOUSE_STATUS);
    }
}

void mouse_change_sample(int rate){ 
    if(rate==10||rate==20||rate==40||rate==60||rate==80||rate==100||rate==200){
        rate=rate;
    }
    else{
        rate = 100;
    }

    mouse_send_cmd();
    if(mouse_wait_before_write()){
        outb(SET_SAMPLE,MOUSE_DATA);
    }
    if(mouse_wait_before_write()){
        outb(rate,MOUSE_DATA);
    }
    mouse_wait_ack();
}

void mouse_change_resol(uint8_t number){
    if(number>0x3) number = 0x3;
    mouse_send_cmd();
    if(mouse_wait_before_write()){
        outb(SET_RESOLUTION,MOUSE_DATA);
    }
    if(mouse_wait_before_write()){
        outb(number,MOUSE_DATA);
    }
    mouse_wait_ack();
}

void mouse_wait_ack(){
    uint8_t info = mouse_read_(MOUSE_DATA);
    while(info!= MOUSE_ACK && info != MOUSE_RESET_BACK){
        info = mouse_read_(MOUSE_DATA);
    }
}

void mouse_reset(){
    mouse_send_cmd();
    mouse_write_(MOUSE_DATA,MOUSE_RESET);
    mouse_wait_ack();
}


int32_t mouse_read (int32_t fd, void* buf, int32_t nbytes){
    if(fd>7 || fd <0 || buf == NULL || nbytes!=2*sizeof(uint32_t)) return -1;
    if(current->window == NULL) return -1;
    uint32_t * ptr = buf;
    float px = current->window->Px;
    float py = current->window->Py;
    float width = current->window->Pw;
    float height = current->window->Ph;
    float x = mouse.x;
    float y = mouse.y; 
    int final_x = -1;
    int final_y = -1;
    if(x<px)  final_x = 0;
    if(x>px+width) final_x = 319;
    if(y<py) final_y = 0;
    if(y>py+height) final_y = 199;
    // change the scale
    x = (x-px)*320/width; 
    y = (y-py)*200/height;
    if(final_x==-1) final_x = (int)x;
    if(final_y==-1) final_y = (int)y;
    if(current->window==window_get_top()){
        ptr[0] = (final_x<<16) + y; // high 16 bits store x, low 16 store y
        ptr[1] = (mouse.left<<16) | mouse.right;
    }else{
        ptr[0] = (final_x<<16) + y; // high 16 bits store x, low 16 store y
        ptr[1] = 0;
    }
    // kprintf("mouse: %d %d %d %d\n", final_x, final_y, mouse.left, mouse.right);
    return 0;
}

int32_t mouse_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1; // syscall
}

int32_t mouse_open (const uint8_t* filename){
    return 0;
}

int32_t mouse_close (int32_t fd){
    return 0;
}





