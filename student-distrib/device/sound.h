#ifndef _SOUND_H_
#define _SOUND_H_

#include "../lib.h"
#define PIT_FRQ 1193180
#define PIT_CH2 0x42
#define PIT_MODEREG 0x43
#define PIT_SOUND 0xB6
// 0xb6 = 1011 0110
// first 10 means chanel 2, 11 means using both high 8 bits and low 8 bits
// as count, 011 means using mode 3 (square wave mode), last 0 means binary mode
#define PIT_INPUT 0x61





typedef struct sound_packet{
    uint16_t type; // 0: play music at a certain frequency 1: mute no sound
    uint16_t frequency;  //if type is 0, this is frequency; if type is 1, this is useless
}sound_packet_t;

void sound_init();
void play_sound ( uint32_t nFrequence );
void nosound ();
void beep();
int32_t sound_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t sound_open(const uint8_t* filename);
int32_t sound_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sound_close(int32_t fd);

#endif
