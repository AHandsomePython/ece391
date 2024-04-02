#include "rtc.h"
#include "lib.h"
#include "interrupt.h"
#include "text_terminal.h"
#include "signal.h"
#include "process.h"

#define TEST_RTC 0

struct rtc_lock rtc_lock; // lock to protect shared variable flag
static unsigned long total_count = 0; // count number of rtc interrupts
static unsigned long cur_freq = 2;
static unsigned long cur_count = 0;
static unsigned long accu_time = 0;
int CP2_TEST = 0; // used to set test for cp2
rtc_buf_t rtc_defult_buf = {2, NULL};
static rtc_buf_t* rtc_buf = &rtc_defult_buf;
/* 
 *   rtc_init
 *   DESCRIPTION: This function is used to initialize RTC device. It firstly turn on IRQ 8, and then
 *                change the interrupt rate to maximum.
 *   INPUTS: none
 *   OUTPUTS: output to port 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: turn on RTC on IRQ 8, change the interrupt rate to maximum.
 */

// Reference : https://wiki.osdev.org/RTC - Turning on IRQ 8 & Change Tnterrupt Rate
void rtc_init(){
    unsigned char pre_input; // store previous input of RTC

    cli();  // set interrupt flag to 0 to disable interrupt
    request_irq(RTC_IRQ, rtc_interrupt, 0);	// register irq handler
    enable_irq(RTC_IRQ);  // enable irq line

//------------------------turn on IRQ 8-------------------------------------------------------------------------------
    outb((uint8_t)(RTC_NMI | RTC_REG_B), RTC_PORT);    // tell RTC port, choose register B(0xB), and disable NMI(0x80)
    pre_input = inb(RTC_CMOS_PORT);	        // read the current value of register B from cmos
    outb((uint8_t)(RTC_NMI | RTC_REG_B), RTC_PORT);    // repeat. because reads will change the register to D
    outb((uint8_t)(pre_input | RTC_ENABLE_INTR), RTC_CMOS_PORT);	// 0X40 ON register B turns on regular interrupt
//------------------------change interrupt rate to max------------------------------------------------------------------------------
    outb((uint8_t)(RTC_NMI | RTC_REG_A), RTC_PORT);	// tell RTC port, choose register A(0xA), and disable NMI(0x80)
    pre_input = inb(RTC_CMOS_PORT);	        // get initial value of register B
    outb( (uint8_t)(RTC_NMI | RTC_REG_A),RTC_PORT);    // repeat. because reads will change the register to D
    pre_input &= BIT_MASK;             // set lower 4 bits of pre value in A to 0. so that new interrupt rate can be written
    outb((uint8_t)(pre_input | 15), RTC_CMOS_PORT);   // write the new rate into A. (6 represents 1024hz)
    sti(); // enable intp
}

/* 
 *   rtc_interrupt
 *   DESCRIPTION: Interrupt handler for rtc.
 *   INPUTS: none
 *   OUTPUTS: output to port 
 *   RETURN VALUE: always 1 
 *   SIDE EFFECTS: when interrupt occurs, set flag to 1, update the total_count
 */
int rtc_interrupt(unsigned int ignore){


    //test_interrupts(); //used for test
    unsigned long flags;
    total_count ++;
    cur_count++;
    if(cur_count % cur_freq == 0){
        accu_time++;
        cur_count = 0;
    }

    if(accu_time==10){
        kernal_send_signal(ALARM, top_pcb->pid);
        accu_time = 0;
    }
    // kprintf("%d,%d\n",cur_count,accu_time);


    
    #if (TEST_RTC == 1)
    //int fre = 2;
    //rtc_frequency(fre<<(total_count/50));
    screen_putc(&screen, '1');
        if(total_count%50 == 0) {
            screen_putc(&screen, '\n');
    }
    #endif

    spin_lock_irqsave(&(rtc_lock.lock),flags); // everytime interrupt occurs, set the flag to 1
    rtc_lock.flag = 1;
    spin_unlock_irqrestore(&(rtc_lock.lock),flags);

    if(rtc_buf!=NULL && rtc_buf->handler!=NULL){
        rtc_buf->handler(rtc_buf);
    }

    outb(RTC_REG_C, RTC_PORT); // RTC requires me to read from register C in interrupt handler. Otherwise interrupt will not occur again
    inb(RTC_CMOS_PORT);

    return 1;


}


/* 
 *   test_power2
 *   DESCRIPTION: This function is used to test if an integer is the power of 2 (from 2 to 1024) 
 *   INPUTS: f -- integer 
 *   OUTPUTS: none 
 *   RETURN VALUE: 1 if power of 2, 0 otherwise.
 *   SIDE EFFECTS: turn on RTC on IRQ 8, change the interrupt rate to maximum.
 */
int test_power2(int f){
    return ((f-1)&f) == 0;
}

/* 
 *   rtc_frequency
 *   DESCRIPTION: This function is used to change rtc interrupt frequency (f must be power of 2 between 2 and 1024)
 *   INPUTS: f-- frequency to be set 
 *   OUTPUTS: output info to RTC 
 *   RETURN VALUE: 0 if success, -1 otherwise 
 *   SIDE EFFECTS: change RTC's interrupt frequency 
 */
int rtc_frequency(int f){ // f is frequency
    int count = 0;
    int index;
    char preinput;
    unsigned long flag;
    if ((f<=1)|(f>1024)) return -1;
    if (!test_power2(f)) return -1; // f must be the power of 2 between 2 and 1024
    while (f!=1){
        f = f>>1;
        count++;
    }// count the power of f

    // new_frequency =  32768 >> (index-1); here we calculate the according index 
    index = 16 - count;
    index &= 0x0F; // only preserve last 4 bits

    cli_and_save(flag); // cli
    outb(RTC_REG_A|RTC_NMI,RTC_PORT);		// select reg A and clear NMI
    preinput=inb(RTC_CMOS_PORT);	// Store the prev value in A
    outb(RTC_REG_A|RTC_NMI,RTC_PORT);		// REpeat
    outb( (preinput & BIT_MASK) | index ,RTC_CMOS_PORT); // write the new frq into low 4 bits
    restore_flags(flag); // sti 
    cur_freq = f;
    return 0;
}

/* 
 *   rtc_frequency
 *   DESCRIPTION: This function is used to read from RTC when interrupt occurs
 *   INPUTS: fd,buf,nbytes -- unused  
 *   OUTPUTS: none
 *   RETURN VALUE: always return 0; 
 *   SIDE EFFECTS: none 
 */

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    // if(buf == NULL) return -1;
    // if(nbytes <= 0) return -1;
    // fd valid>
    unsigned int tmp;
    unsigned int long flags;

    spin_lock_irqsave(&(rtc_lock.lock), flags);
    rtc_lock.flag = 0; //reset to 0
    spin_unlock_irqrestore(&(rtc_lock.lock), flags);

    do{
        spin_lock_irqsave(&(rtc_lock.lock), flags);
        tmp = rtc_lock.flag;
        spin_unlock_irqrestore(&(rtc_lock.lock), flags);
    }while(!tmp); // test if flag == 1, if interrupt occurs

    spin_lock_irqsave(&(rtc_lock.lock), flags);
    rtc_lock.flag = 0; //reset to 0
    spin_unlock_irqrestore(&(rtc_lock.lock), flags);
    // spin_lock(&(rtc_lock.lock));
    // while ((!rtc_lock.flag)); // wait until flag is set to 1 (by interrupt handler)
    // rtc_lock.flag = 0; // then clear the flag 
    // spin_unlock(&(rtc_lock.lock));
    return 0;
}

/* 
 *   rtc_frequency
 *   DESCRIPTION: This function is used to write frequency stored in buffer into rtc. and change its frequency
 *   INPUTS: fd--file descriptor
 *           buf -- buffer storing the fre
 *           nbytes -- number of bytes to be written   
 *   OUTPUTS: none
 *   RETURN VALUE: return number of written bytes if success, -1 otherwise
 *   SIDE EFFECTS: change rtc's frequency 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    if(buf == NULL) return -1;
    if(nbytes != sizeof(int)) return -1; // test invalid parameters
// fd valid 
    cli(); // block intr
    if (rtc_frequency(*((int*)buf)) == 0){ // change the frequency 
        sti();
        return nbytes;
    }
    else{
        sti();
        return -1;
    }
}

/* 
 *   rtc_frequency
 *   DESCRIPTION: This function is used to open RTC file and set frequency to 2hz
 *   INPUTS: filename-- filename 
 *   OUTPUTS: none
 *   RETURN VALUE: return 0 if success, -1 otherwise
 *   SIDE EFFECTS: set rtc's frequency to 2hz
 */
int32_t rtc_open(const uint8_t* filename){
    if (filename == NULL){ // check validity 
        return -1;
    }
    if (rtc_frequency(2) == 0){ // change the frequency 
        return 0;
    }  // when open RTC, default fre is 2hz
    else{
        return -1;
    }


}  

/* 
 *   rtc_close
 *   DESCRIPTION: This function is used to close rtc file. It cannot modify 0 and 1 fd
 *   INPUTS: fd -- file descriptor (ignore)
 *   OUTPUTS: none
 *   RETURN VALUE: return 0 if success, -1 otherwise
 *   SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd) {
    if ((fd == 1)|(fd ==0)|(fd > 7)) return -1; // user cannot close fd 0 and 1, fd can not >7 (not covered in cp2)
    return 0;
}


/* 
 *   rtc_set_buf
 *   DESCRIPTION: This function is used to change the rtc pointer to the current process
 *   INPUTS: buf_ptr -- the rtc pointer to the current process
 *   OUTPUTS: none
 *   RETURN VALUE: return 1
 *   SIDE EFFECTS: change to current process's rtc pointer (set its frequency)
 */
int32_t rtc_set_buf(rtc_buf_t* buf_ptr){
    if(cur_freq != buf_ptr->freq){ // if not equal change to current process's rtc pointer
        rtc_frequency(buf_ptr->freq); //set its freq
    }
    rtc_buf = buf_ptr;
    return 1;
}
