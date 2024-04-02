#include "sound.h"
#include "../lib.h"
#include "../terminal.h"
#include "../rtc.h"
#define SOUND_BIT		3
#define SOUND_SHIFT	8
#define SOUND_SHIFT_HIGH	16
#define SOUND_LOW_MASK 0xFFFF
#define SOUND_MASK		0xFC

// https://wiki.osdev.org/Sound

/* 
 *   sound_init
 *   DESCRIPTION: do nothing
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: none
 */
void sound_init() {
    // Do nothing

}

/* 
 *   play_sound
 *   DESCRIPTION: play a sound along an frequency
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: produce a sound through the speaker
 */
void play_sound ( uint32_t count )  { 
 	uint32_t frq ; 
 	uint8_t temp ;
 
     //set the frequency
 	frq = PIT_FRQ /count; 
 	outb (PIT_SOUND,PIT_MODEREG); 
    
    uint8_t low = (uint8_t)(frq);// 1011 0110
    uint8_t high = (uint8_t)(frq>>SOUND_SHIFT);

 	outb (low,PIT_CH2); 
 	outb (high,PIT_CH2);
 
    //use the speaker to speak
    uint8_t shift = temp|SOUND_BIT;
 	temp = inb(PIT_INPUT); 
  	if (temp != shift){ 
 		outb (shift,PIT_INPUT) ; 
 	} 
    // kprintf("sound_play");

 }

 /* 
 *   nosound
 *   DESCRIPTION: mask the sound
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: none
 */
void nosound ()  { 
 	uint8_t temp = inb(PIT_INPUT)&SOUND_MASK;
 
 	outb ( temp, PIT_INPUT); 
    // kprintf("nosound");
 }

 /* 
 *   beep()
 *   DESCRIPTION: produce a beep
 *   OUTPUTS: none 
 *   RETURN VALUE:  none
 *   SIDE EFFECTS: produce a beep
 */
void beep() {
 	 play_sound(1000);
    int i = 0;
    for(i=0;i<100000000;i++){

    }
    for(i=0;i<100000000;i++){

    }
    for(i=0;i<100000000;i++){

    }
    for(i=0;i<100000000;i++){

    }    
 	 nosound();
    //  screen_printf(&screen,"beep");
          //set_PIT_2(old_frequency);
 }


// void sound_tune(uint8_t row, tune_t tune) {
//     // printf("%d\n", note_tune(row, tune));
//     sound_play(note_tune(row, tune));
// }


 /* 
 *   sound_close
 *   DESCRIPTION: cloes the sound
 *   OUTPUTS: none 
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
int32_t sound_close(int32_t fd){
    //screen_printf(&screen,"check file_close");
    return 0;
}

 /* 
 *   sound_read
 *   DESCRIPTION: read the sound
 *   OUTPUTS: none 
 *   RETURN VALUE:  -1
 *   SIDE EFFECTS: none
 */
int32_t sound_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

 /* 
 *   sound_open
 *   DESCRIPTION: open the sound
 *   OUTPUTS: none 
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
int32_t sound_open(const uint8_t* filename){
    //screen_printf(&screen,"check file_open");
    return 0;
}


/* 
 *   sound_write
 *   DESCRIPTION: write the sound
 *   OUTPUTS: none 
 *   RETURN VALUE:  0
 *   SIDE EFFECTS: none
 */
//
//  buf -> 32 bits
//  | bit 16-31 | bit 0-15 |
//  | freq      | type     |
//
int32_t sound_write(int32_t fd, const void *buf, int32_t nbytes){
    if(buf==NULL || nbytes!=sizeof(uint32_t)) return -1;
    uint32_t val = *((uint32_t*)buf);
    uint32_t type = val & SOUND_LOW_MASK;  // identify the type
    uint32_t freq = (val >> SOUND_SHIFT_HIGH) & SOUND_LOW_MASK; // get the frequency
    switch (type)
    {
    case 0:
        play_sound(freq);
        break;
    case 1:
        nosound();
        break;
    default:
        return -1;
    }
    return 0;
}





