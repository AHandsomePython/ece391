#ifndef MOUSE_H
#define MOUSE_H
#include "../lib.h"

#define MOUSE_DATA     0x60
#define MOUSE_STATUS   0x64
#define MOUSE_ACK      0xFA
#define MOUSE_ENABLE_AUX   0xA8
#define GET_COMPAQ_STA 0x20
#define ENABLE_STREAM  0xF4
#define SET_SAMPLE     0xF3 
#define SET_RESOLUTION 0xE8 
#define MOUSE_CMD            0xD4
#define MOUSE_RESET_BACK     0xAA
#define MOUSE_RESET    0xFF
#define MOUSE_IRQ 12


#define MOUSE_DEC_RATE_X 10
#define MOUSE_DEC_RATE_Y 10

typedef struct mouse{
    uint8_t byte1;
    int8_t x_change;
    int8_t y_change;
    int32_t x;
    int32_t y;
    uint8_t left;
    uint8_t right;
    
}mouse_t;


void mouse_init();
mouse_t* get_mouse();
void mouse_init();
int mouse_handler(unsigned int irq);
int mouse_wait_before_read();
int mouse_wait_before_write();
void mouse_write_(uint8_t port, uint8_t data);
uint8_t mouse_read_(uint8_t port);
void mouse_aux_enable();
void mouse_set_compaq_byte();
void mouse_enable_streaming();
void mouse_send_cmd();
void mouse_change_sample(int rate);
void mouse_change_resol(uint8_t number);
void mouse_wait_ack();
void mouse_reset();

int32_t mouse_read (int32_t fd, void* buf, int32_t nbytes);
int32_t mouse_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t mouse_open (const uint8_t* filename);
int32_t mouse_close (int32_t fd);

#endif
