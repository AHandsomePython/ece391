#ifndef FILESYS_H
#define FILESYS_H

#include "lib.h"
#include "rtc.h"

#define FILESYS_FAIL -1; 
#define FILESYS_SUCCESS 0;

#define BLOCK_SIZE 4096  // 4k
#define FILE_NAME_SIZE 32  //name size up to 32
#define MAX_NUM_DATAB_INDEX 1023 //the file system has data block up to 1023
#define MAX_NUM_DENTRY 63  ////the file system has dentry up to 63
#define DENTRY_SIZE 64    //each dentry 64 size

#define BLOCK_OFF 12
#define BLOCK_MASK 0x0FFF

#define FILEDESC_FLAG_INUSE 1
#define FILEDESC_FLAG_FREE 0

// File structs:
typedef union dentry{
    uint8_t val[DENTRY_SIZE];
    struct{
        char file_name[FILE_NAME_SIZE];
        uint32_t file_type;
        uint32_t inode_index;
        uint8_t reserved[24];
    }__attribute__ ((packed));
}dentry_t;  // the stuct of dentry

typedef struct block{
    uint8_t data[BLOCK_SIZE];
}block_t;   //the struct of the genaral block

typedef union bootb{
    uint8_t val[BLOCK_SIZE];
    struct{
        uint32_t num_dentry;         // 4B
        uint32_t num_inode;             // 4B
        uint32_t num_datab;             // 4B
        uint8_t reserved[52];           // 52B
        dentry_t dentry[MAX_NUM_DENTRY];            // 64 * 63B
    }__attribute__ ((packed));
}bootb_t;  // the struct of the boot block

typedef union inode{
    uint8_t val[BLOCK_SIZE];
    struct{
        uint32_t byte_length;                           // 4B
        uint32_t datab_index[MAX_NUM_DATAB_INDEX];     // 1023 * 4B
    }__attribute__ ((packed));
}inode_t;   // the struct of the inode
 
typedef struct datab{
    uint8_t data[BLOCK_SIZE];             // 4096B
}datab_t;  //the struct of the data block

// File operations:

//initialize the file system
void filesys_init(uint32_t bootb_ptr);

//read the directory entry by its name
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);

//read the directory entry by its index
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);

//read the data from the dentry
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//get the number of the current dentry
inline int32_t filesys_get_num_dentry();

//get the number of the current inode
inline int32_t filesys_get_num_inode();

//get the number of the current data_block
inline int32_t filesys_get_num_datab();

//get the size of data
inline int32_t filesys_get_data_size(uint32_t inode);

// File operation interfaces:
typedef struct file_ops{
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t *filename);
    int32_t (*close)(int32_t fd);
}file_ops_t;


typedef struct filed{
    file_ops_t* ops;
    uint32_t inode_index;
    uint32_t file_position;
    uint32_t flags;
}filed_t;

filed_t defult_fd_arr[8]; //the array of the files

//open the file
int32_t file_open(const uint8_t* filename);

//close the file
int32_t file_close(int32_t fd);

//write into the file
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);

//read the file into the buffer
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) ;

//wirte the directory
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);

//close the directory
int32_t directory_close(int32_t fd);

//open the directory
int32_t directory_open(const uint8_t *fname);

//read the directory into the buf
int32_t directory_read(int32_t fd, void *buf, int32_t nbytes);

#endif
