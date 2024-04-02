#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define BUFSIZE 1024
#define NULL 0

char cur_path[128] = "";


int main ()
{
    int32_t cnt, rval;
	
    uint8_t buf[BUFSIZE];
    ece391_fdputs (1, (uint8_t*)"Starting 391 Shell\n");


    while (1) {
		int flag = 0;
        ece391_fdputs (1, (uint8_t*)"391OS>");
		ece391_fdputs (1, (uint8_t*)cur_path);
		ece391_fdputs (1, " ");
	if (-1 == (cnt = ece391_read (0, buf, BUFSIZE-1))) {
	    ece391_fdputs (1, (uint8_t*)"read from keyboard failed\n");
	    return 3;
	}
	if (cnt > 0 && '\n' == buf[cnt - 1])
	    cnt--;
	buf[cnt] = '\0';

	int b;
	if(ece391_strlen(buf)>3){
		if(buf[0]=='c'&&buf[1]=='d'&&buf[2]==' '&&buf[3]!='.'&&buf[4]!='.'){
			flag = 1;
			for(b =0;b<128;b++){
				if(cur_path[b] == 0) break;
            }
			cur_path[b] = '/';
			b++;
			int count = 3;
			// int cur = 0;
			while(buf[count]!='\0'){
				
				cur_path[b] = buf[count];
				b++;
				count++;
			}
            // cur_path[b] = '/';
			// ece391_fdputs(1,(uint8_t*)cur_path);
		}
		if(buf[0]=='c'&&buf[1]=='d'&&buf[2]==' '&&buf[3]=='.'&&buf[4]=='.'){
			flag = 1;
			char path[128];
			char filename[100];
			ata_split_path(cur_path,path,filename);
			// ata_split_path(cur_path,path,filename);
			ece391_strcpy(cur_path,path);
            // ece391_fdputs(1,(uint8_t*)cur_path);
		}
	}


	if (0 == ece391_strcmp (buf, (uint8_t*)"exit"))
	    return 0;
	if ('\0' == buf[0])
	    continue;
	rval = ece391_execute (buf);
	if(flag == 1) rval = 0;
	if (-1 == rval)
	    ece391_fdputs (1, (uint8_t*)"no such command\n");
	else if (256 == rval)
	    ece391_fdputs (1, (uint8_t*)"program terminated by exception\n");
	else if (0 != rval )
	    ece391_fdputs (1, (uint8_t*)"program terminated abnormally\n");
    }
}




static int32_t get_level_of_path(const char* path){
    if( path==NULL || (path[0] != '/') ) return -1;
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

static int32_t get_arg_by_level(const char* path, char* buf, int32_t level){
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



int32_t ata_split_path(const char* fullpath, char* path, char* filename){
    if(fullpath==NULL||path==NULL||filename==NULL) return -1;
    int i = 0;
    int count = 0;
    while(fullpath[i]!='\0'){
        if(fullpath[i]=='/') count++;
        i++;
    }
    int level = get_level_of_path(fullpath);
    get_arg_by_level(fullpath,filename,level);
    i = 0;
    int j = 0;
    while(count!=0){
        if(fullpath[i]=='/'){
            count--;
        }
        if(count==0) break;
        path[j] = fullpath[i];
        j++;
        i++;
    }
    path[j] = '\0';
    return 0;
}
