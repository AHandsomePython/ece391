#ifndef DISK_FILESYS_H
#define DISK_FILESYS_H

#include "../lib.h"
#include "../filesys.h"

#define ATA_BLOCK_SIZE 512
#define DATABITMAP_NUM 256
#define INODE_NUM 32

typedef union ata_superblock{
    uint8_t val[ATA_BLOCK_SIZE];
    struct{
        uint32_t bootinfo;
        uint32_t start_addr;
        uint32_t num_inode;
        uint32_t num_datab;
        uint32_t inode_off;
        uint32_t datab_off;
        uint32_t datab_bitmap_off;
        uint32_t inode_bitmap;
    }__attribute__ ((packed));
}ata_super_block_t;

typedef struct ata_datab_bitmap{
    uint32_t bitmap[256];
} ata_datab_bitmap;

typedef union ata_inode{
    uint8_t val[ATA_BLOCK_SIZE];
    struct{
        uint32_t type;
        uint32_t byte_length;
        uint16_t datab_index[252];
    }__attribute__ ((packed));
}ata_inode_t;

typedef union ata_dentryblock{
    uint8_t data[ATA_BLOCK_SIZE];
    struct{
        uint32_t type;
        uint32_t num;
        uint8_t  reserved[56];
        dentry_t dentry[7];
    }__attribute__ ((packed));
}ata_dentryblock_t;

typedef struct ata_datab{
    uint8_t data[ATA_BLOCK_SIZE];
}ata_datab_t;

void ata_reformat();
void ata_filesys_init();
int32_t ata_filesys_open(const uint8_t* filename);
int32_t ata_read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_read_data_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_write_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_write_data_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_create_file(const char* path, const char* filename);
int32_t ata_read_dir(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_read_dir_by_path(const char* path, uint32_t offset, uint8_t* buf, uint32_t length);
int32_t ata_create_dir(const char* path, const char* dirname);
int32_t ata_create_dir_by_index(uint32_t inode, const char* dirname);

int32_t ata_file_open(const uint8_t* filename);
int32_t ata_file_close(int32_t fd);
int32_t ata_file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t ata_file_write(int32_t fd, const void *buf, int32_t nbytes);

int32_t ata_dir_open(const uint8_t* filename);
int32_t ata_dir_close(int32_t fd);
int32_t ata_dir_read(int32_t fd, void* buf, int32_t nbytes);
int32_t ata_dir_write(int32_t fd, const void *buf, int32_t nbytes);


#endif
