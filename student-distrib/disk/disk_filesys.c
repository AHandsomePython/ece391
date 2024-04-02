#include "disk_filesys.h"
#include "ata.h"
#include "../process.h"

#define ATA_START_SECTOR 4096
#define ata_read(buf, start, sector) read_ata_st(sector, (uint8_t*)buf, get_ata_device(), (uint32_t)start+ATA_START_SECTOR)
#define ata_write(buf, start, sector) write_ata_st(sector, (uint8_t*)buf, get_ata_device(), (uint32_t)start+ATA_START_SECTOR)

#define INODE_TYPE_FILE 2
#define INODE_TYPE_DIR 1

//
// Stack for path calculation
//
#define STACK_SIZE 32
typedef struct stack{
    int32_t stack[STACK_SIZE];
    int32_t stack_top;
}stack_t;

static int32_t stack_init(stack_t* stack){
    stack->stack_top = 0;
    return 0;
}

static int32_t stack_push(stack_t* stack, int32_t val){
    if(stack->stack_top==STACK_SIZE) return -1;
    stack->stack[stack->stack_top++] = val;
    return stack->stack_top;
}

static int32_t stack_top(stack_t* stack){
    if(stack->stack_top==0) return -1;
    return stack->stack[stack->stack_top-1];
}

static int32_t stack_pop(stack_t* stack){
    if(stack->stack_top==0) return -1;
    return stack->stack[--stack->stack_top];
}

static int32_t stack_empty(stack_t* stack){
    return stack->stack_top==0;
}



//
// Filesys
//
struct ata_filesys_info{
    uint32_t start_addr;
    uint32_t num_inode;
    uint32_t num_datab;
    uint32_t inode_off;
    uint32_t datab_off;
    uint32_t datab_bitmap_off;
    uint32_t inode_bitmap;
    uint32_t datab_bitmap[256];
};

static struct ata_filesys_info info;

//
// Helper Functions:
//
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

static int32_t get_inode_by_index(int index, ata_inode_t* inode){
    ata_read(inode, info.start_addr+info.inode_off+index, 1);
    return 0;
}

static int32_t get_datab_by_index(int index, ata_datab_t* datab){
    ata_read(datab, info.start_addr+info.datab_off+index, 1);
    return 0;
}

static int32_t set_inode_by_index(int index, ata_inode_t* inode){
    ata_write(inode, info.start_addr+info.inode_off+index, 1);
    return 0;
}

static int32_t set_datab_by_index(int index, ata_datab_t* datab){
    ata_write(datab, info.start_addr+info.datab_off+index, 1);
    return 0;
}

static int32_t ata_inode_alloc(){ // return the avai inode number 
    int count;
    static ata_inode_t tmp_inode;
    uint32_t mask = 1;
    for(count=0;count<32;count++){ // go over bitmap
        if(info.inode_bitmap&mask){
            mask = mask<<1;
        }else{
            info.inode_bitmap |= mask; // occupy the inode
            break;
        }
    }
    if(count==32) return -1;
    memset(&tmp_inode, 0, sizeof(ata_inode_t));
    set_inode_by_index(count, &tmp_inode);
    info.num_inode++;
    return count;
}

static int32_t ata_datab_alloc(){ // return the avai data block number 
    int count=0;
    int i,j;
    uint32_t mask = 1;
    uint32_t flag = 0;
    for(i=0;i<DATABITMAP_NUM;i++){
        mask = 1;
        flag = 0;
        for(j=0;j<INODE_NUM;j++){
            if(info.datab_bitmap[i]&mask){
                mask = mask<<1;
            }else{
                info.datab_bitmap[i] |= mask;
                flag = 1;
                break;
            }
        }
        if(flag == 1) break;
    }
    if(i==DATABITMAP_NUM) return -1;
    count = INODE_NUM * i + j;
    info.num_datab++;
    return count;
}

static int32_t ata_get_inode_by_path(const char* path){
    static stack_t stack;
    static ata_inode_t tmp_inode;
    char name[32];
    int level = get_level_of_path(path);
    int i, j;
    int cur_inode = 0;
    stack_init(&stack);
    for(i=0; i<level; i++){
        if(get_arg_by_level(path, name, i+1)==-1) return -1;
        if(strcmp(name, ".")){
            continue;
        }
        if(strcmp(name, "..")){
            stack_pop(&stack);
            if(stack_empty(&stack)){
                cur_inode = 0;
            }else{
                cur_inode = stack_top(&stack);
            }
            continue;
        }
        get_inode_by_index(cur_inode, &tmp_inode);
        if(tmp_inode.type!=INODE_TYPE_DIR){
            return -1;
        }
        ata_dentryblock_t* ptr = (ata_dentryblock_t*) &tmp_inode;
        for(j=0; j<7; j++){
            if(ptr->dentry[j].reserved[0]!=0 && strcmp(name, ptr->dentry[j].file_name)){
                cur_inode = ptr->dentry[j].inode_index;
                stack_push(&stack, cur_inode);
                break;
            }
        }
        if(j==7) return -1;
    }
    return cur_inode;
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

//
// Filesys Operations:
//
static ata_super_block_t superblock;
void ata_reformat(){
    memset(&superblock, 0, sizeof(ata_super_block_t));
    ata_write(&superblock, 1, 1);
    ata_write(&superblock, 2, 1);
    superblock.bootinfo = 0;
    superblock.start_addr = 0;
    superblock.num_inode = 1;
    superblock.num_datab = 0;
    superblock.inode_off = 3;
    superblock.datab_off = 35;
    superblock.datab_bitmap_off = 1;
    superblock.inode_bitmap = 0x1;
    ata_write(&superblock, 0, 1);
    memset(&superblock, 0, sizeof(ata_super_block_t));
    ata_dentryblock_t* ptr = (ata_dentryblock_t*) &superblock;
    ptr->type = 1;
    ata_write(&superblock, 3, 1);
}

void ata_filesys_init(){ 
    ata_read(&superblock, 0, 1);
    info.start_addr = superblock.start_addr;
    info.num_inode = superblock.num_inode;
    info.num_datab = superblock.num_datab;
    info.inode_off = superblock.inode_off;
    info.datab_off = superblock.datab_off;
    info.datab_bitmap_off = superblock.datab_bitmap_off;
    info.inode_bitmap = superblock.inode_bitmap;
    ata_read(info.datab_bitmap, superblock.start_addr+info.datab_bitmap_off, 2);
}

void ata_superblock_restore(){
    ata_read(&superblock, 0, 1);
    superblock.start_addr = info.start_addr;
    superblock.num_inode = info.num_inode;
    superblock.num_datab = info.num_datab;
    superblock.inode_off = info.inode_off;
    superblock.datab_off = info.datab_off;
    superblock.datab_bitmap_off = info.datab_bitmap_off;
    superblock.inode_bitmap = info.inode_bitmap;
    ata_write(&superblock, info.start_addr, 1);
    ata_write(info.datab_bitmap, info.start_addr+info.datab_bitmap_off, 2);
}

// Regular file ops
int32_t ata_read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    static ata_inode_t tmp_inode;
    static ata_datab_t tmp_datab;
    int i,j;
    get_inode_by_index(inode, &tmp_inode);
    if (tmp_inode.type == INODE_TYPE_DIR){
        return -1;
    }
    if (tmp_inode.byte_length <= offset){
        return 0;
    }
    j = offset & 0x1FF;
    get_datab_by_index(tmp_inode.datab_index[(offset) >> 9], &tmp_datab);
    for (i=0;i<length;i++){
        // read temp_datab datab_index
        if (i+offset >= tmp_inode.byte_length){
            return i;
        }
        buf[i] = tmp_datab.data[((i+offset) & 0x1FF)];
        j = ((j+1) & 0x1FF);
        if (j == 0){
            get_datab_by_index(tmp_inode.datab_index[(i+offset) >> 9], &tmp_datab);
        }
    }
    return length;
}

int32_t ata_write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    static ata_inode_t tmp_inode;
    static ata_datab_t tmp_datab;
    if(inode<0 || offset<0 || buf==NULL || length<0) return -1;
    if(get_inode_by_index(inode, &tmp_inode)==-1) return -1;
    if(offset>tmp_inode.byte_length) return -1;
    if(length==0) return 0;
    int32_t pos = offset;
    int32_t i, tmp;
    // if(((tmp_inode.byte_length+(1<<9)-1)>>9)<((offset+(1<<9)-1)>>9))
    if(tmp_inode.byte_length==offset && (tmp_inode.byte_length&0x1FF)==0){
        tmp = ata_datab_alloc();
        if(tmp==-1) return -1;
        tmp_inode.datab_index[tmp_inode.byte_length>>9] = tmp;
    }
    if(get_datab_by_index(tmp_inode.datab_index[pos>>9], &tmp_datab)==-1) return -1;
    for(i=0; i<length; i++, pos++){
        tmp_datab.data[pos&0x1FF] = buf[i];
        if(((pos+1) & 0x1FF) == 0 || ((i+1)==length)){
            if(set_datab_by_index(tmp_inode.datab_index[pos>>9], &tmp_datab)==-1) return -1;
            if(i+1==length) break;
            if(pos+1>=tmp_inode.byte_length){
                tmp = ata_datab_alloc();
                if(tmp==-1) return -1;
                tmp_inode.datab_index[(pos+1)>>9] = tmp;
            }
            if(get_datab_by_index(tmp_inode.datab_index[(pos+1)>>9], &tmp_datab)==-1) return -1;
        }
    }
    tmp_inode.byte_length = tmp_inode.byte_length>=offset+length ? tmp_inode.byte_length : offset+length;
    set_inode_by_index(inode, &tmp_inode);
    return length;
}

int32_t ata_read_data_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length){
    if(buf==NULL || length<0 || offset<0 || path==NULL) return -1;
    int inode_index = ata_get_inode_by_path(path);
    if(inode_index==-1) return -1;
    return ata_read_data(inode_index, offset, buf, length);
}

int32_t ata_write_data_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length){
    if(buf==NULL || length<0 || offset<0 || path==NULL) return -1;
    int inode_index = ata_get_inode_by_path(path);
    if(inode_index==-1) return -1;
    return ata_write_data(inode_index, offset, buf, length);
}

int32_t ata_create_file(const char* path, const char* filename){
    static ata_inode_t tmp_inode;
    ata_dentryblock_t* ptr = (ata_dentryblock_t*) &tmp_inode;
    int32_t inode_index = 0;
    int j;
    int32_t dir_inode = ata_get_inode_by_path(path);
    if(dir_inode==-1) return -1;
    get_inode_by_index(dir_inode, (ata_inode_t*)ptr);
    for(j=0; j<7; j++){
        if(ptr->dentry[j].reserved[0]==0){
            inode_index = ata_inode_alloc();
            if(inode_index==-1) return -1;
            strcpy(ptr->dentry[j].file_name,filename);
            ptr->dentry[j].file_type = 2;
            ptr->dentry[j].inode_index = inode_index;
            // 改变 type
            ptr->dentry[j].reserved[0] = 1;
            ptr->num++;
            set_inode_by_index(dir_inode, (ata_inode_t*) ptr);
            break;
        }
    }
    if(j==7) return -1;
    memset(&tmp_inode, 0, sizeof(ata_inode_t));
    tmp_inode.type = 2;
    set_inode_by_index(inode_index, &tmp_inode);
    return 0;
}



// Directory file ops
int32_t ata_read_dir(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    static ata_dentryblock_t tmp_inode;
    static char namebuf[40];
    if(inode<0 || offset<0 || offset>=7 || buf==NULL || length<0) return -1;
    if(get_inode_by_index(inode, (ata_inode_t*)&tmp_inode)==-1) return -1;
    if(tmp_inode.type !=1) return -1;
    int i;
    int cnt = 0;
    for(i=0; i<7; i++){
        if(tmp_inode.dentry[i].reserved[0]==1){
            if(cnt==offset) break;
            cnt++;
        }
    }
    if(i==7) return 0;
    memset(namebuf, 0, 40);
    memcpy(namebuf, tmp_inode.dentry[i].file_name, length>=FILE_NAME_SIZE ? FILE_NAME_SIZE : length);
    int len = strlen(namebuf);
    namebuf[len] = '\n';
    namebuf[len+1] = '\0';
    strcpy((int8_t*)buf, namebuf);
    return 1;
}

int32_t ata_read_dir_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length){
    if(buf==NULL || length<0 || offset<0 || path==NULL) return -1;
    int inode_index = ata_get_inode_by_path(path);
    if(inode_index==-1) return -1;
    return ata_read_dir(inode_index, offset, buf, length);
}

int32_t ata_create_dir_by_index(uint32_t inode, const char* dirname){
    static ata_inode_t tmp_inode;
    ata_dentryblock_t* ptr = (ata_dentryblock_t*) &tmp_inode;
    int32_t inode_index = 0;
    int j;
    int32_t dir_inode = inode;
    if(dir_inode==-1) return -1;
    get_inode_by_index(dir_inode, (ata_inode_t*)ptr);
    for(j=0; j<7; j++){
        if(ptr->dentry[j].reserved[0]==0){
            inode_index = ata_inode_alloc();
            if(inode_index==-1) return -1;
            strcpy(ptr->dentry[j].file_name, dirname);
            ptr->dentry[j].file_type = 2;
            ptr->dentry[j].inode_index = inode_index;
            // 改变 type
            ptr->dentry[j].reserved[0] = 1;
            ptr->num++;
            set_inode_by_index(dir_inode, (ata_inode_t*) ptr);
            break;
        }
    }
    if(j==7) return -1;
    memset(&tmp_inode, 0, sizeof(ata_inode_t));
    tmp_inode.type = 1;
    set_inode_by_index(inode_index, &tmp_inode);
    return 1;
}

int32_t ata_create_dir(const char* path, const char* dirname){
    static ata_inode_t tmp_inode;
    ata_dentryblock_t* ptr = (ata_dentryblock_t*) &tmp_inode;
    int32_t inode_index = 0;
    int j;
    int32_t dir_inode = ata_get_inode_by_path(path);
    if(dir_inode==-1) return -1;
    get_inode_by_index(dir_inode, (ata_inode_t*)ptr);
    for(j=0; j<7; j++){
        if(ptr->dentry[j].reserved[0]==0){
            inode_index = ata_inode_alloc();
            if(inode_index==-1) return -1;
            strcpy(ptr->dentry[j].file_name, dirname);
            ptr->dentry[j].file_type = 2;
            ptr->dentry[j].inode_index = inode_index;
            // 改变 type
            ptr->dentry[j].reserved[0] = 1;
            ptr->num++;
            set_inode_by_index(dir_inode, (ata_inode_t*) ptr);
            break;
        }
    }
    if(j==7) return -1;
    memset(&tmp_inode, 0, sizeof(ata_inode_t));
    tmp_inode.type = 1;
    set_inode_by_index(inode_index, &tmp_inode);
    return 0;
}

static file_ops_t ata_file_ops = {
    .read = ata_file_read,
    .write = ata_file_write,
    .open = ata_file_open,
    .close = ata_file_close
};

static file_ops_t ata_dir_ops = {
    .read = ata_dir_read,
    .write = ata_dir_write,
    .open = ata_dir_open,
    .close = ata_dir_close
};

int32_t ata_filesys_open(const uint8_t* filename){
    uint32_t flags;
    int32_t fd;
    // dentry_t dentry;
    PCB_t* pcb = current;
    static char path[128];
    static char name[50];
    static ata_inode_t tmp_inode;
    // find the first available file descriptor 
    cli_and_save(flags);
    for(fd=2;fd<FILEARR_SIZE;fd++){
        if(pcb->filearr[fd].flags == FILEDESC_FLAG_FREE){
            break;
        }
    }
    if(fd==FILEARR_SIZE){
        restore_flags(flags);
        return -1; //if no space for new file, return -1
    } 
    int32_t inode = ata_get_inode_by_path((int8_t*)filename);
    if(inode==-1){
        ata_split_path((int8_t*)filename, path, name);
        if(ata_create_file(path, name)==-1){
            restore_flags(flags);
            return -1; //if no space for new file, return -1
        } 
        inode = ata_get_inode_by_path((int8_t*)filename);
        if(inode==-1){
            restore_flags(flags);
            return -1; //if no space for new file, return -1
        } 
    }
    get_inode_by_index(inode, &tmp_inode);
    filed_t* fileobj = &pcb->filearr[fd];
    if(tmp_inode.type==1){
        fileobj->ops = &ata_dir_ops;
    }else if(tmp_inode.type==2){
        fileobj->ops = &ata_file_ops;
    }else{
        restore_flags(flags);
        return -1;
    }
    fileobj->flags = FILEDESC_FLAG_INUSE;
    fileobj->file_position = 0;
    fileobj->inode_index = inode;  //assign the value to this file
    if(fileobj->ops->open(filename)==-1){
        restore_flags(flags);
        return -1;
    } 
    restore_flags(flags);
    return fd;
}

// System Call
int32_t ata_file_open(const uint8_t* filename){
    return 0;
}

int32_t ata_file_close(int32_t fd){
    uint32_t flags;
    cli_and_save(flags);
    ata_superblock_restore();
    restore_flags(flags);
    return 0;
}

int32_t ata_file_read(int32_t fd, void* buf, int32_t nbytes){
    uint32_t flags;
    cli_and_save(flags);

    if (!(current->filearr) || fd < 2 || fd > 7){
        restore_flags(flags);
        return -1;   
    }
    if (buf == NULL || nbytes < 0){
        restore_flags(flags);
        return 0;
    }
    int res;
    uint32_t inode_idx = current->filearr[fd].inode_index;
    uint32_t offset  = current->filearr[fd].file_position;
    res = ata_read_data(inode_idx, offset, buf, nbytes);
    if(res == -1) {
        restore_flags(flags);
        return -1;
    }
    current->filearr[fd].file_position += res;
    restore_flags(flags);
    return res;        
}

int32_t ata_file_write(int32_t fd, const void *buf, int32_t nbytes){
    uint32_t flags;
    cli_and_save(flags);
    if (!(current->filearr) || fd < 2 || fd > 7){
        restore_flags(flags);
        return -1;   
    }
    if (buf == NULL || nbytes < 0){
        restore_flags(flags);
        return 0;
    }
    int res;
    uint32_t inode_idx = current->filearr[fd].inode_index;
    uint32_t offset = current->filearr[fd].file_position;
    res = ata_write_data(inode_idx, offset,(uint8_t*) buf, nbytes);
    if(res == -1) {
        restore_flags(flags);
        return -1;
    }
    current->filearr[fd].file_position += res;
    restore_flags(flags);
    return res; 
}

int32_t ata_dir_open(const uint8_t* filename){
    return 0;
}

int32_t ata_dir_close(int32_t fd){
    uint32_t flags;
    cli_and_save(flags);
    ata_superblock_restore();
    restore_flags(flags);
    return 0;
}

int32_t ata_dir_read(int32_t fd, void* buf, int32_t nbytes){
    uint32_t flags;
    cli_and_save(flags);
    if (!(current->filearr) || fd < 2 || fd > 7){
        restore_flags(flags);
        return -1;   
    }
    if (buf == NULL || nbytes < 0){
        restore_flags(flags);
        return 0;
    }
    int res;
    uint32_t inode_idx = current->filearr[fd].inode_index;
    uint32_t offset  = current->filearr[fd].file_position;
    res = ata_read_dir(inode_idx, offset, buf, nbytes);
    if(res == -1) {
        restore_flags(flags);
        return -1;
    }

    current->filearr[fd].file_position += res;
    if(res==0) {
        restore_flags(flags);
        return 0;
    }
    restore_flags(flags);
    return strlen((char*)buf); 
}

int32_t ata_dir_write(int32_t fd, const void *buf, int32_t nbytes){
    uint32_t flags;
    cli_and_save(flags);
    if (!(current->filearr) || fd < 2 || fd > 7){
        restore_flags(flags);
        return -1;   
    }
    if (buf == NULL || nbytes < 0){
        restore_flags(flags);
        return 0;
    }
    int res;
    uint32_t inode_idx = current->filearr[fd].inode_index;
    // uint32_t offset = current->filearr[fd].file_position;
    res = ata_create_dir_by_index(inode_idx, buf);
    if(res == -1) {
        restore_flags(flags);
        return -1;
    }
    current->filearr[fd].file_position += res;
    restore_flags(flags);
    return res; 
}
