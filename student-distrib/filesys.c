#include "lib.h"
#include "filesys.h"
#include "process.h"
#include "text_terminal.h"
#include "complie_flags.h"

//the struct which contains the file system information
typedef struct filesys_info{
    uint32_t start_addr;
    uint32_t num_inode;
    uint32_t num_datab;
    uint32_t num_dentry;
    dentry_t* dentry_arr;
    inode_t* inode_arr;
    datab_t* datab_arr;
}filesys_info_t;

//initialize the filesys_info_t
static filesys_info_t filesys_info = {0, 0, 0, 0, NULL, NULL, NULL};

// static file_ops_t normal_file_ops = {file_read, file_write, file_open, file_close};

// static file_ops_t dir_ops = {&directory_read, &directory_write, &directory_open, &directory_close};

// static file_ops_t rtc_ops = {&rtc_read, &rtc_write, &rtc_open, &rtc_close};

// static file_ops_t* filetype_to_ops[3] = {&rtc_ops, &dir_ops, &normal_file_ops};


/* 
 *   filesys_init
 *   DESCRIPTION: initialize the file system.
 *                read the information from the boot block into the filesys_info_t
 *   INPUTS: the pointer to the boot block
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will change the value in filesys_info_t
 */
void filesys_init(uint32_t bootb_ptr){
    //corner case
    if(bootb_ptr==NULL) return;

    
    bootb_t* ptr = (bootb_t*)bootb_ptr;
    
    filesys_info.start_addr = bootb_ptr;        //read the start address of the file system
    filesys_info.num_dentry = ptr->num_dentry;  //read the number of dentry of the file system
    filesys_info.num_inode = ptr->num_inode;    //read the number of inode of the file system
    filesys_info.num_datab = ptr->num_datab;    //read the number of data_block of the file system
    filesys_info.dentry_arr = ptr->dentry;      //the first dentry's address 
    filesys_info.inode_arr = (inode_t*)(ptr + 1);  //the first inodes's address
    filesys_info.datab_arr = (datab_t*)(ptr + filesys_info.num_inode + 1);  //the first data_block's address
} 


/* 
 *   filesys_get_num_dentry
 *   DESCRIPTION: get the number of the dentry
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  the number of the dentry (int32_t)
 *   SIDE EFFECTS: none
 */
inline int32_t filesys_get_num_dentry(){
    return filesys_info.num_dentry;
}

/* 
 *   filesys_get_num_inode
 *   DESCRIPTION: get the number of the inode
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  the number of the inode (int32_t)
 *   SIDE EFFECTS: none
 */
inline int32_t filesys_get_num_inode(){
    return filesys_info.num_inode;
}

/* 
 *   filesys_get_num_datab
 *   DESCRIPTION: get the number of the data_block
 *   INPUTS: none
 *   OUTPUTS: none 
 *   RETURN VALUE:  the number of the data_block (int32_t)
 *   SIDE EFFECTS: none
 */
inline int32_t filesys_get_num_datab(){
    return filesys_info.num_datab;
}

/* 
 *   filesys_get_data_size
 *   DESCRIPTION: get the size of the data
 *   INPUTS: the inode you want get size
 *   OUTPUTS: none 
 *   RETURN VALUE:  the size of the data
 *   SIDE EFFECTS: none
 */
inline int32_t filesys_get_data_size(uint32_t inode){
    return filesys_info.inode_arr[inode].byte_length;
}


/*  
 * read_dentry_by_name
 *   DESCRIPTION: search for the fname through the boot block and fill up the input dentry if found
 *   INPUTS: fname  --- the file name we want to search
 *           dentry --- the input dentry we wil fill up 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful
 *                 -1 if fail
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry){
    //corner case
    //check the fname size (should less than 32)
    if(fname==NULL || dentry==NULL||strlen((int8_t*)fname) > 32) return FILESYS_FAIL;
    uint32_t i;
    //look through the boot block to find the file
    for(i=0;i<filesys_info.num_dentry;i++){
        if(!(strncmp((int8_t*)fname, filesys_info.dentry_arr[i].file_name,32))){ //only cmp 0 ~ 31
            *dentry = filesys_info.dentry_arr[i];
            return FILESYS_SUCCESS;
        }
    }
    return FILESYS_FAIL; // if fail, return -1
}

/*  
 * read_dentry_by_index
 *   DESCRIPTION: search for the dentry index through the boot block and fill up the input dentry if found
 *   INPUTS: index  --- the  dentry index we want to search
 *           dentry --- the input dentry we wil fill up 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful
 *                 -1 if fail
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    //corner case
    //check if out of range
    if(dentry==NULL || index>=filesys_info.num_dentry) return FILESYS_FAIL;
    //find the corresponding index
    *dentry = filesys_info.dentry_arr[index];
    return FILESYS_SUCCESS;
}

/*  
 *   read_data
 *   DESCRIPTION: read the data in the inode into the buf
 *                data size up to length and start from the offset
 *   INPUTS: inode --- the index of the inode
 *           offset --- the data starting byte
 *           buf --- read the data into it
 *           length --- the size of data we want to read
 *   OUTPUTS: none
 *   RETURN VALUE: size of data we read
 *                  0 if successful
 *                 -1 if not successful
 *   SIDE EFFECTS: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    //corner case
    if(inode>=filesys_info.num_inode || buf==NULL) return FILESYS_FAIL;
    uint32_t i;
    datab_t* datab_ptr = NULL;
    inode_t* inode_ptr = (filesys_info.inode_arr + inode);
    if(offset >= inode_ptr->byte_length) return 0;
    for(i=0;i<length;i++){
        //check if out of bound
        if(i + offset >= inode_ptr->byte_length) break;
        datab_ptr = (filesys_info.datab_arr + inode_ptr->datab_index[(i + offset) >> BLOCK_OFF]);
        //read into buffer
        buf[i] = datab_ptr->data[(i + offset) & BLOCK_MASK];
    }
    return i;
}

/*  
 * file_open
 *   DESCRIPTION: open the file
 *   INPUTS: fd --- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE:  0 if successful
 *   SIDE EFFECTS: none
 */
int32_t file_open(const uint8_t* filename){
    //screen_printf(&screen,"check file_open");
    return 0;
}

/*  
 * file_close
 *   DESCRIPTION: close the file
 *   INPUTS: fd --- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE:  0 if successful
 *   SIDE EFFECTS: none
 */
int32_t file_close(int32_t fd){
    //screen_printf(&screen,"check file_close");
    return 0;
}

 
/*  
 * file_write
 *   DESCRIPTION: write the file
 *   INPUTS: fd; buffer; nbytes 
 *   OUTPUTS: none
 *   RETURN VALUE: for checkpoint 3.2, just return -1
 *   SIDE EFFECTS: none
 */

#if (FILESYS_WRITABLE == 1)
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
#endif

#if (FILESYS_WRITABLE == 0)
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
#endif

/*  
 *   file_read
 *   DESCRIPTION: read the data of the file into the buffer
 *   INPUTS: fd --- file descriptor
 *           buf --- read data in it
 *           nbytes --- the size of the data we read
 *   OUTPUTS: none
 *   RETURN VALUE: the bytes of data we read
 *   SIDE EFFECTS: none
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    //screen_printf(&screen,"check file_read");
    /* file usable array range: 2-7 */
    if (!(current->filearr) || fd < 2 || fd > 7){
        return -1;   
    }
    if (buf == NULL || nbytes < 0)
        return 0;
    int res;
    uint32_t inode_idx = current->filearr[fd].inode_index;
    uint32_t offset  = current->filearr[fd].file_position;
    res = read_data(inode_idx , offset, buf, nbytes);
    if(res == -1) return -1;
    current->filearr[fd].file_position += res;

    return res;
}


/*  
 *   directory_write
 *   DESCRIPTION: for checkpoint 3.2, just return -1
 *   INPUTS: fd -- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
#if (FILESYS_WRITABLE == 1)
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
#endif


#if (FILESYS_WRITABLE == 0)
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
#endif


/*  
 *   directory_close
 *   DESCRIPTION: close the directory
 *   INPUTS: fd -- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t directory_close(int32_t fd){
    return 0;
}

/*  
 *   directory_open
 *   DESCRIPTION: open the directory
 *   INPUTS: fd -- file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
int32_t directory_open(const uint8_t* filename){
    return 0;
}






/* 
 *   directory_read
 *   DESCRIPTION: read the directory
 *   INPUTS: fd; buf; nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if successful
 *                 -1 if fail
 *   SIDE EFFECTS: none
 */
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes){
    // screen_printf(&screen," check directory_read");
    dentry_t dentry;            
    int8_t* flie_name_p;          
    uint32_t file_name_length;       
    if(current->filearr[fd].file_position == filesys_info.num_dentry) return 0;
    //read the diectory by the position of the file
    if(read_dentry_by_index(current->filearr[fd].file_position, &dentry) != 0){
        return -1;
    }
        
    flie_name_p = (int8_t*)(&(dentry.file_name[0]));
    file_name_length = strlen(flie_name_p);
    uint32_t i;
    //clear the buffer
    for(i = 0; i < 32; i ++){
        ((int8_t*)buf)[i] = '\0';
    }

    //check the length of the bytes we read
    if(nbytes < file_name_length){
        file_name_length = nbytes;
    }   
    strncpy((int8_t*)buf, flie_name_p, 32);

    //move to next directoy position
    current->filearr[fd].file_position += 1;

    return file_name_length;
}
