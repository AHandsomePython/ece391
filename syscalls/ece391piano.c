#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200

#define BLACK 0
#define WHITE 0x3F
#define RED 0x30

#define BLACK_HEIGHT 97
#define WHITE_HEIGHT 147
#define BLACK_WIDTH 11
#define WHITE_WIDTH 22

unsigned char window_buf[WINDOW_WIDTH * WINDOW_HEIGHT];

void draw_rect(int start_x, int start_y, int width, int height, unsigned char color){
    int i, j;
    for(i=0; i<height; i++){
        for(j=0; j<width; j++){
            window_buf[(start_y + i) * WINDOW_WIDTH + (start_x + j)] = color;
        }
    }
}

void draw_piano(){
    int i, j, x, y, index;
    draw_rect(0, 0, 22*14, 147, WHITE);
    for(i=0; i<14; i++){
        for(j=0; j<=WHITE_HEIGHT; j++){
            x = i*WHITE_WIDTH;
            y = j;
            index = y*WINDOW_WIDTH + x;
            window_buf[index] = BLACK;
        }
        for(j=0; j<=WHITE_HEIGHT; j++){
            x = i*WHITE_WIDTH+1;
            y = j;
            index = y*WINDOW_WIDTH + x;
            window_buf[index] = BLACK;
        }
    }
    draw_rect(22-5, 0, 10, 97, BLACK);
    draw_rect(22*2-5, 0, 10, 97, BLACK);
    draw_rect(22*4-5, 0, 10, 97, BLACK);
    draw_rect(22*5-5, 0, 10, 97, BLACK);
    draw_rect(22*6-5, 0, 10, 97, BLACK);
    draw_rect(22*8-5, 0, 10, 97, BLACK);
    draw_rect(22*9-5, 0, 10, 97, BLACK);
    draw_rect(22*11-5, 0, 10, 97, BLACK);
    draw_rect(22*12-5, 0, 10, 97, BLACK);
    draw_rect(22*13-5, 0, 10, 97, BLACK);

    draw_rect(10, 170, 20, 20, RED);
}

int check_range(int x, int y, int rx, int ry, int rw, int rh){
    if(x>=rx && y>=ry && x<rx+rw && y<ry+rh) return 1;
    return 0;
}

static int white_key_table[14] = {0, 2, 4, 5, 7, 9, 11, 12+0, 12+2, 12+4, 12+5, 12+7, 12+9, 12+11};
static int black_key_table[14] = {1, 3, 4, 6, 8, 10, 11, 12+1, 12+3, 12+4, 12+6, 12+8, 12+10, 12+11};

int get_key(int x, int y){
    if(x<0 || y<0 || x>=22*14 || y>=147) return -1;
    if(y>=97){
        return white_key_table[x/22];
    }else{
        return black_key_table[(x-11)/22];
    }
}

static uint32_t key_to_freq[24] = {262, 277, 294, 311, 329, 349, 370, 392, 415, 440, 466, 494, 
                                    523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 987};

int main(){
    ece391_fdputs(1, "hello piano\n");
    int fd_sound = ece391_open((uint8_t*)"sound");
    if(fd_sound==-1) return 1;
    int fd_mouse = ece391_open((uint8_t*)"mouse");
    if(fd_mouse==-1){
        ece391_close(fd_sound);
        return 1;
    } 
    int fd_window = ece391_open((uint8_t*)"window");
    if(fd_window==-1){
        ece391_close(fd_sound);
        ece391_close(fd_mouse);
        return 1;
    }
    uint32_t mouse[2];
    int x, y, right, left, key;
    int is_mute = 0;
    uint32_t sound_packet;
    draw_piano();
    ece391_write(fd_window, window_buf, WINDOW_WIDTH*WINDOW_HEIGHT);
    while(1){
        ece391_read(fd_mouse, mouse, 2*sizeof(uint32_t));
        // ece391_printf("xxx\n");
        y = mouse[0] & 0xFFFF;
        x = (mouse[0] >> 16) & 0xFFFF;
        right = mouse[1] & 0xFFFF;
        left = (mouse[1] >> 16) & 0xFFFF;
        if(left){
            is_mute = 0;
            key = get_key(x, y);
            // ece391_printf("key = %d\n", key);
            if(x==0){
                continue;
            }
            if(key!=-1){
                sound_packet = (key_to_freq[key]<<16);
                ece391_write(fd_sound, &sound_packet, sizeof(uint32_t));
            }else{
                sound_packet = 1;
                ece391_write(fd_sound, &sound_packet, sizeof(uint32_t));
                if(check_range(x, y, 10, 170, 20, 20)){
                    break;
                }
            }
        }else{
            sound_packet = 1;
            if(is_mute) continue;
            ece391_write(fd_sound, &sound_packet, sizeof(uint32_t));
            is_mute = 1;
        }
    }
    ece391_close(fd_window);
    ece391_close(fd_mouse);
    ece391_close(fd_sound);
    return 0;
}
