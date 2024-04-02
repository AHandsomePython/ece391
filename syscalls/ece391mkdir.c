#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#define NULL 0

int32_t get_level_of_path(const char* path){
    int i = 0;
    int32_t count = 0;
    while(path[i]!='\0'){
        if(path[i] == '/'){
            if(path[i+1]!= '\0' && path[i+1]!= '/'){
                count++;
                i++;
            }
            else{
                i++;
            }
        }   
        else{
            i++;
        }
    }
    return count;
}

int32_t get_arg_by_level(const char* path, char* buf, int32_t level){
    int32_t cor = get_level_of_path(path);
    if(level>cor||level<0||buf==NULL||path==NULL) return -1;
    int i = 0;
    int j = 0;
    while(level!=0){
        if(path[i] == '/'){
            level--;
            i++;
        }
        else{
            i++;
        }
    }
    while(path[i]!='\0' && path[i]!='/'){
        buf[j] = path[i];
        j++;
        i++;
    }
    buf[j] = '\0';
    return 0;
}


char* my_strncpy(char* dest, const char* src, int num)
{
	char* tmp = dest;
	while (num && (*dest++ = *src++))
	{
		num--;
	}
	if (num)
	{
		while (num--)
		{
			*dest++ = '\0';
		}
	}
	return tmp;
}

int my_strlen(const char *str)
{
    int len=0;
    if(str ==NULL){
    	return -1;
	}
    while(*str++!='\0') 
         len++;
    return len;
}

int my_strcmp(const char* str1, const char* str2)
{
	int ret = 0;
	while(!(ret=*(unsigned char*)str1-*(unsigned char*)str2) && *str1)
	{
		str1++;
		str2++;
	}
 
 
	if (ret < 0)
	{
		return -1;
	}
	else if (ret > 0)
	{
		return 1;
	}
	return 0;
}

int main(){
    // ata_createdir(const char* path, const char* dirname)
    uint8_t buf[128];
    uint8_t dir[128];
    uint8_t path[128];
    int i;
    for (i=0;i<128;i++){
        path[i] = '\0';
        dir[i] = '\0';
        buf[i] = '\0';
    }
    int fd;
    if (0 != ece391_getargs (buf, 128)) {
        ece391_fdputs (1, (uint8_t*)"could not read argument\n");
	    return 3;
    }
    int32_t level = get_level_of_path((const char*) buf);
    get_arg_by_level((const char*)buf, (char*)dir, level);
    int dir_len = my_strlen((const char*)dir);
    int buf_len = my_strlen((const char*)buf);
    my_strncpy((char*)path, (const char*)buf, buf_len - dir_len);
    if ( my_strcmp((const char*)path, "disk0/") != 0){
        path[buf_len - dir_len-1] = '\0';
    }
    fd= ece391_open((const uint8_t*)path);
    if (fd == -1){
        ece391_fdputs (1, (uint8_t*)"could not open path\n");
	    return 3;
    }
    if (-1 == ece391_write(fd, (const void*)dir, dir_len+1)){
        ece391_fdputs (1, (const uint8_t*)"could not write dir\n");
	    return 3;
    }
    return 0;
}

