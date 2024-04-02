#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "i8259.h"
#include "interrupt.h"
#include "paging.h"
#include "text_terminal.h"
#include "filesys.h"
#include "syscall.h"
#include "process.h"
#include "./GUI/screen.h"
#include "./GUI/window.h"
#include "./GUI/mouse.h"
#include "./GUI/gui.h"
#include "pit.h"
#include "disk/ata.h"
#include "disk/disk_filesys.h"


#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");
const uint8_t* commandC;

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


// /* Checkpoint 1 tests */

// /* IDT Test - Example
//  * 
//  * Asserts that first 10 IDT entries are not NULL
//  * Inputs: None
//  * Outputs: PASS/FAIL
//  * Side Effects: None
//  * Coverage: Load IDT, IDT definition
//  * Files: x86_desc.h/S
//  */
// int idt_test(){
// 	TEST_HEADER;
// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) && 
// 			(idt[i].offset_31_16 == NULL)){
// 			result = FAIL;
// 		}
// 	}
// 	return result;
// }

// // add more tests here
// /* 
//  * divide_by_zero_exception_test
//  *   DESCRIPTION: test the exception: divided by zero
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: raise an exception
//  */
// int divide_by_zero_exception_test() {
// 	TEST_HEADER;
// 	int a = 1;
// 	int b = 0;
// 	int c = a/b;
// 	return c;
// }
// /* 
//  * system_call_test
//  *   DESCRIPTION: test the system call
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: call a system call to see the result
//  */
// int system_call_test(){
// 	TEST_HEADER;
// 	asm volatile("int $0x80");
// 	return PASS;
// }
// /* 
//  *   invalid_opcode_test
//  *   DESCRIPTION: test the exception: invalid opcode
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: raise an exception
//  */
// void invalid_opcode_test() {
// 	TEST_HEADER;
// 	asm ("rsm");
// }
// /* 
//  *   page_fault_exception_test
//  *   DESCRIPTION: test the page fault
//  * 				  use the null ptr to test
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: raise an exception
//  */
// void page_fault_exception_test(){
// 	TEST_HEADER;
// 	int* p = NULL;
// 	int a;
// 	a = *p;
// }

// /* 
//  *   page_overflow_test
//  *   DESCRIPTION: test the page fault
//  * 				  the kernel page only has 4MB, we use the address > 4MB to test
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: raise an exception
//  */
// void page_overflow_test(){
// 	TEST_HEADER;
// 	int* p = (int*)(0x800000 + 0x100000);
// 	int a;
// 	a= *p;
// }
// /* 
//  *   paging_vga_test
//  *   DESCRIPTION: test the VGA page
//  *   INPUTS: none
//  *   OUTPUTS: 1
//  *   RETURN VALUE: 1
//  *   SIDE EFFECTS: none
//  */
// int paging_vga_test(){
// 	TEST_HEADER;
// 	int *a = (int *)0x000B8000;
// 	int b;
// 	b = *a;
// 	return 1;
// }
// /* 
//  *   paging_kernal_test
//  *   DESCRIPTION: test the kerbel page
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: 1
//  *   SIDE EFFECTS: none
//  */
// int paging_kernal_test(){
// 	TEST_HEADER;
// 	int *a = (int *)((1<<22)+12);
// 	int b;
// 	b = *a;
// 	return 1;
// }
// /* 
//  *   paging_struct_test
//  *   DESCRIPTION: test the page struct to check if PDE & PTE are valid
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: flag!=0 
//  *   SIDE EFFECTS: none
//  */
// int paging_struct_test(){
// 	TEST_HEADER;
// 	int i;
// 	int flag = 1;
// 	for(i=0;i<PAGE_DIR_ENTRY_NUM;i++){
// 		flag = flag && page_directory[i].val;
// 	}
// 	for(i=0;i<PAGE_TAB_ENTRY_NUM;i++){
// 		flag = flag && page_table0[i].val;
// 	}
// 	return flag!=0;
// }

// /* garbage_input_test: It will call functions with invalid input parameters and test their return values
//  * 
//  * Inputs: None
//  * Outputs: none
//  * Side Effects: None
//  * Coverage: i8259.c interrupt.c keyboard.c 
//  */

// void garbage_input_test(){
// 	i8259_enable_irq(100);
// 	i8259_disable_irq(90);
// 	i8259_send_eoi(1000);
// 	if(request_irq(-20,NULL,0) == 0) printf("garbage_input_test for request_irq passed! \n");
// 	free_irq(-100);
// 	disable_irq(30);
// 	enable_irq(203);
// 	if(mask_and_act(-2) == 0) printf("garbage_input_test for mask_and_act passed! \n");
// 	if(do_IRQ(NULL) == 0) printf("garbage_input_test for do_IRQ passed! \n");
// 	if(keyboard_buf_put(NULL,0) == 0) printf("garbage_input_test for keyboard_buf_put passed! \n");
// 	if(keyboard_buf_get(NULL) == 0) printf("garbage_input_test for keyboard_buf_get passed! \n");
// 	if(keyboard_buf_seek(NULL) == 0) printf("garbage_input_test for keyboard_buf_seek passed! \n");
// 	if(keyboard_buf_init(NULL) == 0) printf("garbage_input_test for keyboard_buf_init passed! \n");
// 	if(keyboard_set_buf(NULL) == 0) printf("garbage_input_test for keyboard_set_buf passed! \n");
// }


// add more tests here

/* Checkpoint 2 tests */


/* 
 *   list_all_files
 *   DESCRIPTION: list all files in the file system 
 * 				  includes file_name, file_type, file_size
 *   INPUTS: none
 *   OUTPUTS: file name; file type; file size
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

// void list_all_files(){
	
// 	int8_t buf[32 + 1];	//create a buffer to set the >32 bytes invalid
// 	buf[32] = 0;
// 	int i;
// 	dentry_t dentry;
// 	screen_printf(&screen,"\n");
// 	//look through the file index thourgh the boot block
// 	for(i=0;i<filesys_get_num_dentry();i++){
// 		//get the dentry
// 		read_dentry_by_index(i, &dentry);
// 		//copy the file name into the buffer
// 		strncpy(buf, dentry.file_name,32);
// 		uint32_t length=strlen(buf);
// 		uint32_t field_length = 35;
// 		//set the space between the "file name" and file name
// 		screen_printf(&screen,"file_name:");   
// 		uint32_t i =0;
// 		for(i = 0; i<field_length-length; i++){
// 		 	screen_printf(&screen," ");
// 		 }
// 		screen_printf(&screen,"%s, file_type: %d  file_size:  %d\n",buf,dentry.file_type, filesys_get_data_size(dentry.inode_index));
// 		// screen_puts(&screen, dentry.file_name);
// 		// screen_putc(&screen, '\n');
// 	}
// }

// /* 
//  *   test_ls
//  *   DESCRIPTION: list all files in the file system 
//  * 				  includes file_name, file_type, file_size
//  *   INPUTS: none
//  *   OUTPUTS: file name; file type; file size
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// void test_ls(){
// 	TEST_HEADER;
// 	int32_t fd, cnt;
// 	uint8_t buf[33];
// 	dentry_t dentry;

// 	buf[32] = '\0';
// 	fd = open((uint8_t*)".");
	
// 	while (0 != (cnt = read (fd, buf, 32))) {
// 		//check if read the name of dentry successfully
//         if (-1 == cnt) {
// 	        printf ((int8_t*)"directory entry read failed\n");
// 	        return;
// 	    }
// 		//according to the file ame read the data of the file
// 		read_dentry_by_name((int8_t*)buf, &dentry);
// 		uint32_t length=strlen((int8_t*)buf);
// 		uint32_t field_length = 35;
// 		uint32_t i =0;
// 		//print relevant information
// 		screen_printf(&screen,"filename: ");
// 		for(i = 0; i<field_length-length; i++){
// 		 	screen_printf(&screen," ");
// 		 }
		
// 		screen_printf(&screen,"%s, file_type: %d  file_size:  %d\n",buf,dentry.file_type, filesys_get_data_size(dentry.inode_index));


//     }

// 	return;

// }

// /* 
//  *   read_file_by_name
//  *   DESCRIPTION: read the data and print on the screen 
//  *   INPUTS: the file name we want to read
//  *   OUTPUTS: the data in this file
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// static char buf[100000];
// int32_t read_file_by_name(char* s){
// 	int j;
// 	for(j = 0; j < 100000; j++){
//             ((int8_t*)buf)[j] = '\0';
//     } // clear the buffer
// 	int length;
// 	dentry_t dentry;
// 	//corner case
// 	if(-1 == read_dentry_by_name(s, &dentry)){
// 		screen_printf(&screen,"invalid file name! ");
// 		return FILESYS_FAIL;
// 	}
	
// 	length = filesys_get_data_size(dentry.inode_index);
// 	//read data into the buffer
// 	read_data(dentry.inode_index, 0, (uint8_t*)buf, length);
// 	buf[length] = 0;

// 	int i ;
// 	for(i = 0 ; i<100000;i++){
// 		// don't print null bytes
// 		if(buf[i]!=0){
// 			screen_putc(&screen, buf[i]);
// 		}
// 		 //putc(buf[i]);
// 	}
// 	char buffer[32+1];
// 	buffer[32]=0;
// 	//just print the pre 32 bytes of the file name
// 	strncpy(buffer,dentry.file_name,32);
// 	kprintf("\nfile_name: %s\n",  buffer);
// 	return FILESYS_SUCCESS;
// }



// /* 
//  *   rtc_test
//  *   DESCRIPTION: test open/close/read/write func of RTC and see whether its freq is changed
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none 
//  *   SIDE EFFECTS: test rtc, printing "1"s with different freq on screen
//  */
// void rtc_test(){
// 	uint8_t* filename;
// 	int i,j,ignore; //used for loop
// 	rtc_open(filename); //open rtc 
// 	int buf = 2;
// 	int nbyte = 4;
// 	for(i=0;i<10;i++){
// 		buf = 2<<i;
// 		for(j=0;j<1000000000;j++){
// 			ignore++; // this big number is to cost time for printing
// 		}
// 		rtc_read(2,&buf,nbyte); //read 
// 		rtc_write(2,&buf,nbyte); // write 
// 	}
// 	rtc_close(2);
// }

// void garbage_input_test_cp2(){ // garbage input 

// 	if(rtc_open(NULL) == -1) screen_printf(&screen,"garbage_input_test for rtc_open passed! \n");
// 	if(rtc_close(1) == -1) screen_printf(&screen,"garbage_input_test for rtc_close passed! \n");
// 	if(rtc_write(0,NULL,0) == -1) screen_printf(&screen,"garbage_input_test for rtc_write passed! \n");
// 	if(read_file_by_name(NULL) == -1) screen_printf(&screen,"garbage_input_test for read_file_by_name passed! \n");
// }


// /* Checkpoint 3 tests */


// /* 
//  *   print_small_file
//  *   DESCRIPTION: read the data and print on the screen , test "read txt"
//  *   INPUTS: none
//  *   OUTPUTS: the data in this file
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */

// void print_small_file() {
// 	TEST_HEADER;
// 	uint8_t buf[500];
// 	uint32_t j;
// 	for(j = 0; j < 500; j++){
//          ((int8_t*)buf)[j] = '\0';
//     } // clear the buffer
// 	int32_t fd_cur,i,result;
// 	i = 0;
// 	//open the file
// 	fd_cur = open((uint8_t*)"frame0.txt");
// 	if(fd_cur ==-1){
// 		kprintf("fail opening frame0.txt\n");
// 		return;
// 	}
// 	//read the data into the buffer
// 	read(fd_cur,buf,500);

// 	//print the buffer
// 	for(i = 0 ; i<500;i++){
// 		// don't print null bytes
// 		if(buf[i]!=0){
// 			screen_putc(&screen, buf[i]);
// 		}
// 		 //putc(buf[i]);
// 	}
// 	result=write(fd_cur, buf, 500);
// 	if(result!=-1){
// 		screen_printf(&screen,"lala");
// 	}
// 	screen_printf(&screen,"\n");
// 	screen_printf(&screen,"file_name: frame0.txt");
// 	close(fd_cur);
// }


// /* 
//  *   print_exe_file
//  *   DESCRIPTION: read the data and print on the screen , test "read exe"
//  *   INPUTS: none
//  *   OUTPUTS: the data in this file
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// void print_exe_file() {
// 	TEST_HEADER;
// 	uint8_t buf[6000];
// 	uint32_t j;
// 	for(j = 0; j < 6000; j++){
//          ((int8_t*)buf)[j] = '\0';
//     } // clear the buffer
// 	int32_t fd_cur,i,result;
// 	i = 0;
// 	//open the file
// 	fd_cur = open((uint8_t*)"ls");
// 	if(fd_cur ==-1){
// 		kprintf("fail opening ls\n");
// 		return;
// 	}
// 	//read the data into the buffer
// 	read(fd_cur,buf,6000);
// 	//print the buffer
// 	for(i = 0 ; i<6000;i++){
// 		// don't print null bytes
// 		if(buf[i]!=0){
// 			screen_putc(&screen, buf[i]);
// 		}
// 		 //putc(buf[i]);
// 	}
// 	result=write(fd_cur, buf, 6000);
// 	if(result!=-1){
// 		screen_printf(&screen,"lala");
// 	}
// 	screen_printf(&screen,"\n");
// 	screen_printf(&screen,"file_name: ls");
// 	close(fd_cur);
	
// }

// /* 
//  *   print_large_file
//  *   DESCRIPTION: read the data and print on the screen , test "read large file"
//  *   INPUTS: none
//  *   OUTPUTS: the data in this file
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// void print_large_file() {
// 	TEST_HEADER;
// 	//kprintf("check print_large_file");
// 	uint8_t buf[6000];
// 	//kprintf("1\n");
// 	// uint32_t j;
// 	// for(j = 0; j < 5000; j++){
//     //     ((int8_t*)buf)[j] = '\0';
//     // } // clear the buffer
// 	int32_t fd_cur,i,result;
// 	//kprintf("2\n");
// 	i = 0;
// 	//open the file
// 	fd_cur = open((uint8_t*)"verylargetextwithverylongname.txt");
// 	//kprintf("3\n");
// 	if(fd_cur ==-1){
// 		kprintf("fail opening verylargetextwithverylongname.txt\n");
// 		return;
// 	}
// 	//kprintf("4\n");
// 	//read the data into the buffer
// 	read(fd_cur,buf,6000);
// 	//kprintf("5\n");
// 	//print the buffer
// 	for(i = 0 ; i<6000;i++){
// 		// don't print null bytes
// 		if(buf[i]!=0){
// 			screen_putc(&screen, buf[i]);
// 		}
// 		 //putc(buf[i]);
// 	}
// 	//kprintf("6\n");
// 	result=write(fd_cur, buf, 6000);
// 	if(result!=-1){
// 		screen_printf(&screen,"lala");
// 	}
// 	screen_printf(&screen,"\n");
// 	screen_printf(&screen,"file_name: verylargetextwithqverylongname.txt");
// 	close(fd_cur);
	
// }
	
// 	// char buf[1024];
// 	// int count = read(fd, &buf, 1024);
// 	// //printf("BUF:\n%s",buf);
// 	// write(1, buf, count);



// /* 
//  *   garbage_input_test_cp3
//  *   DESCRIPTION: test the invalid input of our functions
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// void garbage_input_test_cp3(){ // garbage input 

// 	if(PCB_init(NULL) == -1) screen_printf(&screen,"garbage_input_test for PCB_init passed! \n");
// 	if(free_PCB_pos(10) == -1) screen_printf(&screen,"garbage_input_test for free_PCB_pos passed! \n");

// 	if(sys_execute(NULL) == -1) screen_printf(&screen,"garbage_input_test for sys_execute passed! \n");
// 	if(sys_read(10,NULL,0) == -1) screen_printf(&screen,"garbage_input_test for sys_read passed! \n");
// 	if(sys_write(-1,NULL,0) == -1) screen_printf(&screen,"garbage_input_test for sys_write passed! \n");
// 	if(sys_open(NULL) == -1) screen_printf(&screen,"garbage_input_test for free_PCB_pos passed! \n");
// 	if(sys_close(-1) == -1) screen_printf(&screen,"garbage_input_test for sys_close passed! \n");
// 	if(sys_getargs(NULL,0) == -1) screen_printf(&screen,"garbage_input_test for sys_getargs passed! \n");

// }

// /* 
//  *   test_empty_function
//  *   DESCRIPTION: test the function that not used by shell
//  *   INPUTS: none
//  *   OUTPUTS: none
//  *   RETURN VALUE: none
//  *   SIDE EFFECTS: none
//  */
// void test_empty_function(){
// 	char test_buf[3];
// 	int i;
// 	char test_name[3] = {'h', 'h', 'h'};
// 	terminal_open((const uint8_t*)test_name);
// 	terminal_close(1);
// 	keyboard_open((const uint8_t*)test_name);
// 	keyboard_close(0);
// 	terminal_buf_clear();
// 	keyboard_buf_put(&terminal_buffer, 'a');
// 	keyboard_buf_put(&terminal_buffer, 'b');
// 	keyboard_buf_put(&terminal_buffer, 'c');
// 	terminal_read(1, test_buf, 3);
// 	for(i=0;i<3;i++){
// 	kprintf("%c\n", test_buf[i]);
// }

// }
// /* Checkpoint 4 tests */
// /* Checkpoint 5 tests */





/* Test suite entry point */
void launch_tests(){
	// clear();
	// screen_init(&screen);
	// for(i=0;i<SCREEN_ROWS;i++){
	// 	for(j=0;j<SCREEN_COLS;j++){
	// 		*(uint8_t *)(0xB8000 + ((j * SCREEN_COLS + i) << 1)) = ' ';
	// 	}
	// }
	// int32_t i;
    // for (i = 0; i < 25 * 80; i++) {
    //     *(uint8_t *)(0xB8000 + (i << 1)) = ' ';
    //     *(uint8_t *)(0xB8000 + (i << 1) + 1) = 0x7;
    // }
	// *(uint8_t *)(0xB8000 + ((0) << 1)) = ' ';
	// int i,j,k,n;
	// screen_init(&screen);
	// for(n=0;n<2;n++){
	// 	for(i=0;i<SCREEN_ROWS;i++){
	// 		for(j=0;j<SCREEN_COLS;j++){
	// 			for(k=0;k<1000000;k++);
	// 			screen_put_char(&screen, '0'+n);
	// 		}
	// 	}
	// }
	// screen_del_char(&screen);
	//screen_put_char(&screen,'0' + strcmp("111","111"));
	// filesys_test1();
	//screen_clear(&screen);
//--------------------CP1-------------------------------------------

	//test_interrupts();
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("paging_struct_test", paging_struct_test());
	// TEST_OUTPUT("paging_vga_test", paging_vga_test());
	// TEST_OUTPUT("paging_kernal_test", paging_kernal_test());
	// garbage_input_test();
	//system_call_test();
	// divide_by_zero_exception_test();
	// invalid_opcode_test();
	// page_overflow_test();
	// page_fault_exception_test();

//---------------------CP2----------------------------------------
	////rtc_test();
	//list_all_files();
	// screen_clear(&screen);
	//read_file_by_name("ls");
	//read_file_by_name(".");
	//read_file_by_name("sigtest");
	//read_file_by_name("shell");
	//read_file_by_name("grep");
	// read_file_by_name("syserr");
	// read_file_by_name("rtc");
	// read_file_by_name("fish");
	// read_file_by_name("counter");
	// read_file_by_name("pingpong");
	// read_file_by_name("cat");
	 //read_file_by_name("frame0.txt");
	 //read_file_by_name("verylargetextwithverylongname.txt");
	 //read_file_by_name("verylargetextwithverylongname.tx");
	 //read_file_by_name("ls");
	// read_file_by_name("testprint");
	// read_file_by_name("created.txt");
	//  read_file_by_name("frame1.txt");
	 //read_file_by_name("hello");
	//garbage_input_test_cp2();

//------------------------------cp3-------------------------------------------------
	//uint8_t** screen_start = NULL;
	//void* buf = NULL;
	//halt(1);
	//execute (commandC);
	//read (2 ,buf ,3);
	//write (2, buf, 3);
	//open ( commandC);
	//close (2);
	//getargs (buf, 3);
	//vidmap (screen_start);
	//set_handler(1, buf);
	//sigreturn ();
	
	// screen_clear(&screen);
	// execute("shell");
	//print_small_file();
	//print_exe_file();
	// print_large_file();
	//test_read_dir();
	//test_ls();
	//execute("shell");
	//garbage_input_test_cp3();



//----------------------------bonus--------------------------------
	// screen_printf(&screen,"11111111111111111");
	// modex_init();
	// // GUI_window_init(&window);
	// // int i;
	// // GUI_window_clear(&window, 40);
	// // GUI_window_draw_font(&window, 0, 0, 100, 100, 'A', 10);
	// // GUI_window_draw_font(&window, 100, 100, 8, 16, 'B', 10);
	// // GUI_window_draw_font(&window, 310, 170, 7, 14, 'a', 10);
	// // screen_display(cur_screen_buf);
	// // GUI_window_change_pos(&window, 0, 0, WINDOW_DEFULT_WIDTH, WINDOW_DEFULT_HEIGHT);
	// // screen_display(cur_screen_buf);
	// GUI_init();
	// terminal_driver_init();
	// terminal_t* terminal = terminal_alloc();
	// terminal->occupied = GUI_window_alloc();
	// GUI_window_init(terminal->occupied, "terminal");
	// terminal->ops->init(terminal);
	// mouse_init();
	// // screen_display(cur_screen_buf);
	// uint32_t i, j;
	// for(i=0; i<1000000; i++){
	// 	for(j=0; j<1000000; j++);
	// 	terminal->ops->putc(terminal, '0' + i % 10);
	// 	// screen_display(cur_screen_buf);
	// 	// if(i==500){
	// 	// 	GUI_window_change_pos(&terminal, 0, 0, WINDOW_DEFULT_WIDTH*4/5, WINDOW_DEFULT_HEIGHT*4/5);
	// 	// }else{
	// 	// 	if(i==1000){
	// 	// 		GUI_window_change_pos(&terminal, 100, 100, WINDOW_DEFULT_WIDTH, WINDOW_DEFULT_HEIGHT);
	// 	// 	}
	// 	// }
	// 	// terminal->ops->delc(terminal);
	// 	// for(j=0;j<100000000;j++);
	// }
	// GUI_terminal_printf(&terminal, "xxxxxxxxxxxxxxxxxxxxx agaaghjwswegw !!! %d\nafsfasfag\n\n\nASSSS", 12);
	
	// pit_init(20);
	// execute("shell");
	
}

