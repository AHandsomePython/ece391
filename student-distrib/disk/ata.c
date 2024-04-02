#include "ata.h"
#include "../lib.h"
#include "../terminal.h"
#include "../interrupt.h"

// reference https://wiki.osdev.org/ATA_PIO_Mode 
ata_dev_t ata_dev;


int ata1_handler(unsigned int ignore){
    return 0;
}

int ata2_handler(unsigned int ignore){
    return 0;
}

/*
IDENTIFY command
To use the IDENTIFY command, select a target drive by sending 0xA0 for the master drive, or 0xB0 for the slave, to the "drive select" IO port. 
On the Primary bus, this would be port 0x1F6. 
Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5). 
Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7). Then read the Status port (0x1F7) again. 
If the value read is 0, the drive does not exist. 
For any other value: poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears. 
Because of some ATAPI drives that do not follow spec, 
at this point you need to check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) 
to see if they are non-zero. 
    If so, the drive is not ATA, and you should stop polling. 
    Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, 
    or until bit 0 (ERR, value = 1) sets.
At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0). 
Read 256 16-bit values, and store them.
return 0 for success, -1 for fail

reference https://stackoverflow.com/questions/57944636/what-does-rep-insw-do 
*/
int ata_indentify(ata_dev_t* ata){
    outb(0xA0, ATA1_DATA6);
    outb(0, ATA1_DATA2);
    outb(0, ATA1_DATA3);
    outb(0, ATA1_DATA4);
    outb(0, ATA1_DATA5);
    outb(ID_CMD, ATA1_DATA7);
    int res = inb(ATA1_DATA7);
    if (res == 0){
        printf("the drive does not exist\n");
        return -1;
    } else {
        // poll the port
        while ((res & 0x80) != 0){
            res = inb(ATA1_DATA7);
        }
    }
    res = inb(ATA1_DATA4);
    if (res != 0){
        printf("not an ATA\n");
        return -1;
    }
    res = inb(ATA1_DATA5);
    if (res != 0){
        printf("not an ATA\n");
        return -1;
    }
    res = inb(ATA1_DATA7);
    while ((res & 0x08) == 0){
        if ((res&0x01) == 1){
            printf("error while reading ATA\n");
            return -1;
        }
    }
    uint16_t temp_buf[256];
    // copy data from 0x1F0 for 256 times and store them
    asm volatile(
        "movl %0, %%edx                     \n\t"
        "movl %1, %%edi                     \n\t"
        "movl $256, %%ecx                   \n\t" 
        "rep insw                           \n\t"
        :
        : "r"(ATA1_DATA0), "r"(temp_buf)
        : "edx", "edi", "ecx"
    );
    ata->lba28 = ((temp_buf[61]) << 16) + temp_buf[60];
    if ((temp_buf[83] & 0x200) == 0){
        // bit 10 is not set, doesn't support lba48
        ata->lba48l = 0; 
        ata->lba48h = 0;
    }
    else {
        ata->lba48l =  ((temp_buf[101]) << 16) + temp_buf[100];
        ata->lba48h =  ((temp_buf[103]) << 16) + temp_buf[102];
    }
    return 1;
}


/*
For non-ATAPI drives, the only method a driver has of resetting a drive after a major error is to do a "software reset"
on the bus. 
Set bit 2 (SRST, value = 4) in the proper Control Register for the bus. 
This will reset both ATA devices on the bus. 
Then, you have to clear that bit again, yourself. 
The master drive on the bus is automatically selected. 
ATAPI drives set values on their LBA_LOW and LBA_HIGH IO ports, 
but are not supposed to reset or 
even terminate their current command.
reference: https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command 
translate ASM to C function
*/

void ata_reset(ata_dev_t* ata){
    int32_t data_base;
    int32_t cmd_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
        cmd_base = ATA1_CMD0;
    }
    else {
        data_base = ATA2_DATA0;
        cmd_base = ATA2_CMD0;
    }
    // data_base+7
    outb(RESSET_CMD, (data_base+7));
    outb(0, (data_base+7));
    // outb(RESSET_CMD, (cmd_base+0));
    // outb(0, (cmd_base+0));
    //400ns delay
    ata_delay(ata);
    int res = inb((data_base+7));
    // check RSY and RDY
    while ((res & 0xC0) != 0x40){
        //want BSY clear and RDY set 
        res = inb((data_base+7));
    }
    return;
}

/*
There is a similar problem after writing the Command Register, with the ERR/DF bits. T
hey are two slightly different kinds of errors that can terminate a command. 
BSY and DRQ will be cleared, but ERR or DF 
remain set until just after you write a new command to the Command Register. 
If you are using polling (see below), you should account for the fact that 
your first four reads of the Status Register, after sending your command byte, 
may have the ERR or DF bits still set accidentally. 
(If you are using IRQs, the Status will always be correct by the time the IRQ is serviced.)
*/
void ata_delay(ata_dev_t* ata){
    int i;
    int data_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
    }
    else {
        data_base = ATA2_DATA0;
    }
    for (i=0;i<4;i++){
        inb(data_base+7);
    }
    return;
}


/*
; do a singletasking PIO ATA read
; inputs: ebx = # of sectors to read, edi -> dest buffer, esi -> driverdata struct, ebp = 4b LBA
; Note: ebp is a "relative" LBA -- the offset from the beginning of the partition
; outputs: ebp, edi incremented past read; ebx = 0
; flags: zero flag set on success, carry set on failure (redundant)
reference: https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command 
translate ASM to C function
*/
int read_ata_st(int sector, uint8_t* buf, ata_dev_t* ata, int lba){
    int data_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
    }
    else {
        data_base = ATA2_DATA0;
    }
    if (sector < 0){
        ata_reset(ata);
        // ???
        // test sector?
        return 0;
    }
    if (sector > 0x3fffff){
        printf(" reading larger than 2GB, error occurs\n");
        return -1;
    }
    int res = inb((data_base+7));
    //check the BSY and DRQ bits -- both must be clear ??????????????
    if ((res & 0x88 )!= 0){
        // ????
        ata_reset(ata);
    }
    /*
    ; preferentially use the 28bit routine, because it's a little faster
    ; if ebp > 28bit or esi.stLBA > 28bit or stLBA+ebp > 28bit or stLBA+ebp+ebx > 28bit, use 48 bit
    */
    // offset + base
    uint32_t abs_lba = lba+ata->stdlba;
    if (abs_lba > 0xfffffff){
        printf("doen't support 48 bits now\n");
        return -1;
    }
    int i=0;
    // 1 sector read each time, 1 sector is 256 16bit value, or 512 8 bit value
    // abs_lba+1, buf + 512
    for (i=0;i<sector;i++){
        if (0 != pio28_read(1, buf, ata, abs_lba)){
            return -1;
        }
        buf+=512;
        abs_lba+=1;
    }
    return 1;
}

int pio28_read(int sector, uint8_t* buf, ata_dev_t* ata, int abs_lba){
    int32_t data_base;
    int32_t cmd_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
        cmd_base = ATA1_CMD0;
    }
    else {
        data_base = ATA2_DATA0;
        cmd_base = ATA2_CMD0;
    }

    // drive select
    // bits 24 to 28 of LBA | master/slave flag  | 0xe0
    int slave_bit = 1;
    if (ata->master_f == 1){
        slave_bit = 0;
    }
    outb( (0xE0 | (slave_bit << 4) | ((abs_lba >> 24) & 0x0F)) , data_base+6);
    // Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): outb(0x1F1, 0x00)
    // outb(0, data_base+1)

    outb(1, (data_base+2)); // read 1 sector everytime

    // load lba low, mid high
    outb((uint8_t)(abs_lba&0xff), (data_base+3));
    outb((uint8_t)((abs_lba >> 8)&0xff), (data_base+4));
    outb((uint8_t)((abs_lba >> 16)&0xff), (data_base+5));

    // Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
    outb(READ_SECTOR_CMD, (data_base+7));

    /*
    ; ignore the error bit for the first 4 status reads -- ie. implement 400ns delay on ERR only
    ; wait for BSY clear and DRQ set
    */
    int i,res;
    int ready = 0;
    res = inb(data_base+7);
    for (i=0;i<4;i++){
        if ((res & 0x80) == 0){
            if ((res & 0x08) != 0){
                ready = 1;
                break;
            }
        }
        res = inb(data_base+7);
    }

    // ; need to wait some more -- loop until BSY clears or ERR sets (error exit if ERR sets)
    if (ready != 1){
        res = inb(data_base+7);
        while ((res & 0x80) !=0 ){
            res = inb(data_base+7);
            if ((res & 0x21) != 0){
                // check DF and ERR
                return -1;
            }
        }
    }

    // ; if BSY and ERR are clear then DRQ must be set -- go and read the data
    // ; read from data port (ie. 0x1f0)
    uint16_t* temp_buf = (uint16_t*)buf;
    // copy data from 0x1F0 for 256 times and store them
    asm volatile(
        "movl %0, %%edx                     \n\t"
        "movl %1, %%edi                     \n\t"
        "movl $256, %%ecx                   \n\t" 
        "rep insw                           \n\t"
        :
        : "r"(data_base), "r"(temp_buf)
        : "edx", "edi", "ecx"
    );
    /*
    Note for polling PIO drivers: After transferring the last uint16_t of a 
    PIO data block to the data IO port, give the drive a 400ns delay to reset its DRQ bit 
    (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
    */
    ata_delay(ata);

    //check ERR and DF 
    res = inb(data_base+7);
    if ((res & 0x21) != 0){
        return -1;
    }
    return 0;
}


/*
; do a singletasking PIO ATA read
; inputs: ebx = # of sectors to read, edi -> dest buffer, esi -> driverdata struct, ebp = 4b LBA
; Note: ebp is a "relative" LBA -- the offset from the beginning of the partition
; outputs: ebp, edi incremented past read; ebx = 0
; flags: zero flag set on success, carry set on failure (redundant)
reference: https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command 
translate ASM to C function
*/
int write_ata_st(int sector, uint8_t* buf, ata_dev_t* ata, int lba){
    int data_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
    }
    else {
        data_base = ATA2_DATA0;
    }
    if (sector < 0){
        ata_reset(ata);
        return 0;
    }
    if (sector > 0x3fffff){
        printf(" reading larger than 2GB, error occurs\n");
        return -1;
    }
    int res = inb((data_base+7));
    //check the BSY and DRQ bits -- both must be clear 
    if ((res & 0x88 )!= 0){
        ata_reset(ata);
    }
    /*
    ; preferentially use the 28bit routine, because it's a little faster
    ; if ebp > 28bit or esi.stLBA > 28bit or stLBA+ebp > 28bit or stLBA+ebp+ebx > 28bit, use 48 bit
    */
    // offset + base
    uint32_t abs_lba = lba+ata->stdlba;
    if (abs_lba > 0xfffffff){
        printf("doen't support 48 bits now\n");
        return -1;
    }
    int i=0;
    // 1 sector read each time, 1 sector is 256 16bit value, or 512 8 bit value
    // abs_lba+1, buf + 512
    for (i=0;i<sector;i++){
        if (0 != pio28_write(1, buf, ata, abs_lba)){
            return -1;
        }
        buf+=512;
        abs_lba+=1;
    }
    return 1;
}

int pio28_write(int sector, uint8_t* buf, ata_dev_t* ata, int abs_lba){
    int32_t data_base;
    int32_t cmd_base;
    if (ata->master_f == 1){
        data_base = ATA1_DATA0;
        cmd_base = ATA1_CMD0;
    }
    else {
        data_base = ATA2_DATA0;
        cmd_base = ATA2_CMD0;
    }

    // drive select
    // bits 24 to 28 of LBA | master/slave flag  | 0xe0
    int slave_bit = 1;
    if (ata->master_f == 1){
        slave_bit = 0;
    }
    outb( (0xE0 | (slave_bit << 4) | ((abs_lba >> 24) & 0x0F)) , data_base+6);
    // Send a NULL byte to port 0x1F1, if you like (it is ignored and wastes lots of CPU time): outb(0x1F1, 0x00)
    // outb(0, data_base+1)

    outb(1, (data_base+2)); // read 1 sector everytime

    // load lba low, mid high
    outb((uint8_t)(abs_lba&0xff), (data_base+3));
    outb((uint8_t)((abs_lba >> 8)&0xff), (data_base+4));
    outb((uint8_t)((abs_lba >> 16)&0xff), (data_base+5));

    // Send the "READ SECTORS" command (0x20) to port 0x1F7: outb(0x1F7, 0x20)
    outb(WRITE_SECTOR_CMD, (data_base+7));

    /*
    ; ignore the error bit for the first 4 status reads -- ie. implement 400ns delay on ERR only
    ; wait for BSY clear and DRQ set
    */
    int i,res;
    int ready = 0;
    res = inb(data_base+7);
    for (i=0;i<4;i++){
        if ((res & 0x80) == 0){
            if ((res & 0x08) != 0){
                ready = 1;
                break;
            }
        }
        res = inb(data_base+7);
    }

    // ; need to wait some more -- loop until BSY clears or ERR sets (error exit if ERR sets)
    if (ready != 1){
        res = inb(data_base+7);
        while ((res & 0x80) !=0 ){
            res = inb(data_base+7);
            if ((res & 0x21) != 0){
                // check DF and ERR
                return -1;
            }
        }
    }

    // ; if BSY and ERR are clear then DRQ must be set -- go and read the data
    // ; read from data port (ie. 0x1f0)
    uint16_t* temp_buf = (uint16_t*)buf;
    // copy data from 0x1F0 for 256 times and store them
    // There must be a tiny delay between each OUTSW output uint16_t. 
    // A jmp $+2 size of delay. 
    // Make sure to do a Cache Flush (ATA command 0xE7) after each write command completes.
    asm volatile(
        "movl %0, %%edx                     \n\t"
        "movl %1, %%esi                     \n\t"
        "movl $256, %%ecx                   \n\t" 
        "rep outsw                          \n\t"
        :
        : "r"(data_base), "r"(temp_buf)
        : "edx", "edi", "ecx"
    );
    
    /*
    Note for polling PIO drivers: After transferring the last uint16_t of a 
    PIO data block to the data IO port, give the drive a 400ns delay to reset its DRQ bit 
    (and possibly set BSY again, while emptying/filling its buffer to/from the drive).
    */
    ata_delay(ata);

    outb(CA_FLUSH, (data_base+7));
    //check ERR and DF 
    res = inb(data_base+7);
    if ((res & 0x21) != 0){
        return -1;
    }
    return 0;
}





/*
;ATA PI0 33bit singletasking disk read function (up to 64K sectors, using 48bit mode)
; inputs: bx = sectors to read (0 means 64K sectors), edi -> destination buffer
; esi -> driverdata info, dx = base bus I/O port (0x1F0, 0x170, ...), ebp = 32bit "relative" LBA
; BSY and DRQ ATA status bits must already be known to be clear on both slave and master
; outputs: data stored in edi; edi and ebp advanced, ebx decremented
; flags: on success Zero flag set, Carry clear
reference: https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command 
translate ASM to C function
*/
int pio48_read(){
    return 0;
}

void ata_init(){
    request_irq(ATA1_IRQ, ata1_handler, 0);
    enable_irq(ATA1_IRQ);
    request_irq(ATA2_IRQ, ata2_handler, 0);
    enable_irq(ATA2_IRQ);
    ata_dev.master_f = 1;
    ata_dev.par_length = 0x8000000;
    // ata_dev.par_length = 0x800000;
    ata_dev.lba28 = 0;
    ata_dev.stdlba = 0;
    ata_indentify(&ata_dev);
}

inline ata_dev_t* get_ata_device(){
    return &ata_dev;
}

void ata_test(){
    // int i;
    // ata_init();
    
    // uint8_t buf1[512];
    // uint8_t buf2[512];
    // for (i=0;i<512;i++){
    //     buf1[i] = i%256;
    // }
    
    // write_ata_st(1, buf1, &ata_dev, 6000);
    // read_ata_st(1, buf2, &ata_dev, 6000);
    // for (i=0;i<512;i++){
    //     printf( "%d ", buf2[i]);
    //     if(i%10==0) printf("\n");
    // }
    // printf("%d\n", ata_dev.lba28);
}

