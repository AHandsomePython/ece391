#include "syscall.h"
#include "lib.h"
#include "process.h"
#include "rtc.h"
#include "filesys.h"
#include "text_terminal.h"
#include "paging.h"
#include "exception.h"
#include "./GUI/window.h"
#include "./GUI/mouse.h"
#include "terminal.h"
#include "./device/sound.h"
#include "./GUI/gui.h"
#include "scheduling.h"
#include "complie_flags.h"
#include "./memoryalloc/malloc.h"
#include "terminal.h"
#include "./disk/disk_filesys.h"
#include "disk/ata.h"

#define TEST_EXEC 0
#define TEST_HALT 0
#define TEST_FILE_SYSCALL 0

#define EXCEPTION_HALT_RETVAL 256

// #define EXEC_MAX_NUM_SHELL 3
static int num_of_shell = 0;

// static int num_of_sched_trie = 0;

#define CMD_MAX_LEN 50


#define get_process_memory_from_pos(pcb_pos) ((uint32_t)(pcb_pos * PAGE_4MB_VAL + (PAGE_4MB_VAL << 1)))


static char* terminal_names[TERMINAL_NUM] = {"Terminal 0", "Terminal 1", "Terminal 2"};

static file_ops_t rtc_ops = {rtc_read, rtc_write, rtc_open, rtc_close};
static file_ops_t dir_ops = {directory_read, directory_write, directory_open, directory_close};
static file_ops_t file_ops = {file_read, file_write, file_open, file_close};
static file_ops_t window_ops = {GUI_window_read, GUI_window_write, GUI_window_open, GUI_window_close};
static file_ops_t mouse_ops = {mouse_read, mouse_write, mouse_open, mouse_close};
static file_ops_t sound_ops = {sound_read, sound_write, sound_open, sound_close};
// static file_ops_t tty_ops;

static uint32_t halt_status = 0;

#if (TEST_EXEC == 1 || TEST_HALT == 1)
void delay(){
    uint32_t i;
    uint32_t cnt = 0;
    for(i=0;i<200000000;i++) cnt++;
}
#endif

int32_t get_num_of_shell(){
    return num_of_shell;
}

uint32_t set_virtual_memory(uint32_t pcb_pos){
    if(pcb_pos>=MAX_PROCESS_NUM) return -1;
    uint32_t pde_index = get_dir_entry(PROGRAM_START_VIRTUAL_ADDR);
    page_directory[pde_index].val = 0;
    page_directory[pde_index].present = 1;
    page_directory[pde_index].size = 1;
    page_directory[pde_index].rw = 1;
    page_directory[pde_index].user_supervisor = 1;
    page_directory[pde_index].val |= get_process_memory_from_pos(pcb_pos);
    // page_directory[pde_index].val = PAGEF_PRESENT | PAGEF_RW | PAGEF_SIZE | (get_process_memory_from_pos(pcb_pos) & ALIGNED_ADDR_MASK);
    reload_tlb();
    return 0;
}


/*  
 * sys_halt
 *   DESCRIPTION: syetem call: terminate a process
 *   INPUTS: status :  return value to the parent 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t sys_halt (uint8_t status){
    PCB_t* pcb = current;       // pcb pointer
    PCB_t* parent = pcb->parent;
    uint32_t flags;
    uint32_t i = 0;
#if (MULTI_THREAD == 1)
    if(pcb->thread_flag==1){
        kill_thread(0); // 0 means kill itself
    }
    for(i=0; i<MAX_THREAD_NUM_FOR_EACH; i++){
        if(pcb->threads[i]!=-1){
            kill_thread(pcb->threads[i]);
        }
    }
#endif
    cli_and_save(flags);
#if (TEST_HALT == 1)
    kprintf("\nsys_halt Start:\n");
#endif
    //
    // Part 1 Free PCB
    //
    for(i=0; i<FILEARR_SIZE; i++){
        if(pcb->filearr[i].flags & FILEDESC_FLAG_INUSE){
            if(pcb->filearr[i].ops!=NULL) pcb->filearr[i].ops->close(i);
        }
    }
    // free fd all the files 
    if(pcb->window!=NULL) GUI_window_free(pcb->window);
    pcb->window = NULL;
    halt_status = (uint32_t)status;
    if(pcb->parent==NULL){
        num_of_shell = 0;
        sched_free_pid(pcb);
        if(pcb->terminal->occupied!=NULL){
            GUI_window_exit(pcb->terminal->occupied);
            GUI_window_free(pcb->terminal->occupied);
        } 
        terminal_free(pcb->terminal);
        free_PCB_pos(pcb->PCB_pos);     // free current pid in the process id array
        top_process_set(sched_get_available_pid());
        restore_flags(flags);
        // uint32_t i;
        // for(i=0; i<1000000000; i++);
        sys_execute((uint8_t*)"shell");
    }
    // Set sched info
    sched_set(pcb->parent);
    // Set top procress
    if(top_process_get()==pcb){
        top_process_set(parent->pid);
    }
    free_PCB_pos(pcb->PCB_pos);     // free current pid in the process id array
    uint32_t parent_esp = parent->esp;      // restore parent esp and ebp
    uint32_t parent_ebp = parent->ebp;
#if (TEST_HALT == 1)
    kprintf("Part 1 Finished, Info below:\n    current = 0x%x\n    parent = 0x%x\n    parent_esp = 0x%x    parent_ebp = 0x%x\n"
    , (uint32_t)pcb, (uint32_t)parent, parent_esp, parent_ebp);
    delay();
#endif

    //
    // Part 2 Restore Parent TSS
    //
    uint32_t parent_pos = parent->PCB_pos;
    tss.ss0 = KERNEL_DS;            // set tss parameters
    tss.esp0 = get_kernel_stack_from_pos(parent_pos);
#if (TEST_HALT == 1)
    kprintf("Part 2 Finished, parent_pos = 0x%x, tss.ss0 = 0x%x, tss.esp0 = 0x%x\n", parent_pos, tss.ss0, tss.esp0);
    delay();
#endif

    //
    // Part 3 Restore Parent Paging
    //
    if(set_virtual_memory(parent_pos)==-1){
        restore_flags(flags);
        return -1;
    }
#if (TEST_HALT == 1)
    kprintf("Part 3 Finished, tss.ss0 = 0x%x, tss.esp0 = 0x%x\n", tss.ss0, tss.esp0);
    delay();
#endif

    //
    // Part 4 Free PCB
    //
    if(pcb->shell_flag){
        // 待修改
        // if(pcb->terminal->occupied!=NULL){
        //     GUI_window_exit(pcb->terminal->occupied);
        //     GUI_window_free(pcb->terminal->occupied);
        // } 
        // terminal_free(pcb->terminal);
        num_of_shell--;
        // kprintf("num of shell: %d\n", num_of_shell);
    }
    for(i=0; i<FILEARR_SIZE; i++){  //close the relavant files
        if(pcb->filearr[i].flags == FILEDESC_FLAG_INUSE){
            pcb->filearr[i].ops->close(i);
        }
        pcb->filearr[i].file_position = 0;
        pcb->filearr[i].flags = FILEDESC_FLAG_FREE;
        pcb->filearr[i].inode_index = -1;
        pcb->filearr[i].ops = NULL;    //reset the variable to initial value
    }
#if (TEST_HALT == 1)
    kprintf("Part 4 Finished\n");
    delay();
#endif    

    //
    // Part 5 Jump to execute
    //
    // asm volatile(
    //     "movl %0, %%esp         \n\t"
    //     "movl %1, %%ebp         \n\t"
    //     "jmp halt_return_addr   \n\t"
    //     :
    //     : "r" (parent_esp), "r" (parent_ebp)
    //     : "eax"
    // );  // return status to execute

    asm volatile(
        "movl %0, %%esp         \n\t"
        "movl %1, %%ebp         \n\t"
        "movl %2, %%eax         \n\t"
        "pushl %3               \n\t"
        "popfl                  \n\t"
        "jmp *%%eax             \n\t"
        :
        : "r" (parent_esp), "r" (parent_ebp), "r" (parent->eip), "r" (flags)
        : "eax"
    );

    return 0;
}



/* get_process_cmd(const uint8_t* command, uint8_t* cmd, uint8_t* argv)
 *  Functionality: It's used to handle the input command and seperate it into
                   cmd and arguments. if more than one arguments is input, space
                   will be added between arguments
 *  Arguments: command -- input command to be handled
               cmd -- string used to hold cmd part in command
               argv --  string used to hold arg part in command
 *  Return: number of arguments in command
 */
int32_t get_process_cmd(const uint8_t* command, uint8_t* cmd, uint8_t* argv){
    if (command==NULL || cmd==NULL || argv==NULL){ // input check
        return -1;
    }
    uint32_t command_index = 0;
    uint32_t cmd_index = 0;
    uint32_t cmd_flag = 0;
    uint32_t arg_index = 0;
    uint32_t arg_flag = 0; // indicate whether there's arg
    uint32_t space_flag = 0; // used to add space between args
    uint32_t argc = 0; // number of arg
    uint32_t argc_flag = 0; // used to calculate argc

    while(command[command_index]!='\0' && command[command_index]==' '){
            command_index++;
    } // skip first spaces

    while(command[command_index]!='\0' && command[command_index]!=' '){
            cmd[cmd_index] = command[command_index];
            cmd_index++;
            if((cmd_index+1)>CMD_MAX_LEN) return -1; // check max length
            command_index++;
            cmd_flag = 1; // indicate cmd detected
    } // read cmd
    
    while(command[command_index]!='\0' && command[command_index]==' '){
            command_index++;
    } // skip spaces

    while(command[command_index]!='\0'){ // fill in arguments
        if(space_flag==1){
            argv[arg_index] = ' '; // space bewteen args
            arg_index++;
            if((arg_index+1)>ARGV_MAX_LEN) return -1; // check max length
            space_flag = 0;
            argc_flag = 0;
        }

        if(command[command_index] == ' '){
            command_index++;
        }

        while(command[command_index] != ' ' && command[command_index]!='\0'){
            argv[arg_index] = command[command_index];
            arg_flag = 1;
            space_flag=1;
            command_index++;
            arg_index++;
            if((arg_index+1)>ARGV_MAX_LEN) return -1; // check max length
            if(argc_flag==0){
                argc++;
                argc_flag = 1; // used to count the number of arguments
            }
        }
    }
    cmd[cmd_index] = '\0';
    argv[arg_index] = '\0'; // add '\0' at the end of string
    return argc;
}


/*  
 * sys_execute
 *   DESCRIPTION: system call: load, set and execute a new program
 *   INPUTS: const uint8_t* command -- the command to be executed
 *   OUTPUTS: none
 *   RETURN VALUE: 256 if occur exception
 *                 0 to 255 if execute successfully
 *                 -1 if failed
 *   SIDE EFFECTS: none
 */
int32_t sys_execute(const uint8_t* command){
    if(command == NULL) return -1;
    uint32_t flags;
    cli_and_save(flags);
#if (TEST_EXEC == 1)
    kprintf("\nsys_execute Start:\n");
#endif
    //
    //  Part 1 Parse Args
    //
    uint8_t cmd[CMD_MAX_LEN + 1];
    uint32_t argc = 0;
    uint8_t argv[ARGV_MAX_LEN + 1];
    argc = get_process_cmd(command, cmd, argv); //get the argumets
    if(argc == -1){
        restore_flags(flags);
        return -1;
    }
    // if(strcmp((const char*)cmd, "shell") && num_of_shell>=EXEC_MAX_NUM_SHELL){
    //     restore_flags(flags);
    //     return -1;
    // }
#if (TEST_EXEC == 1)
    kprintf("Part 1 Finished, argc = %d, argv = \"%s\"\n", argc, argv);
#endif
    
    //
    // Part 2 Check EXE
    //
    //check if the exe file exists in the file system
    dentry_t dentry;
    uint32_t entry;
    uint8_t entry_point[4];    //store the correct magic numbers
    uint8_t four_magics[4]; //store the four magic numbers
    uint8_t correct_magics[4]={0x7F,0x45,0x4c,0x46}; //first four magic numbers
    if (read_dentry_by_name((int8_t*)cmd, &dentry) == -1)
    {
        restore_flags(flags);
        return -1;
    }    
    //The first 4 bytes of the exe file is 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46
    if(read_data(dentry.inode_index, 0, four_magics, 4) == -1){
        restore_flags(flags);
        return -1;
    }
    int i=0;
    while(i<4){
        if(four_magics[i]!=correct_magics[i]){
            restore_flags(flags);
            return -1;
        }
        i++;
    }

    //get the entry point n which is stored as a 4-byte unsigned integer in bytes 24-27 of the executable
    if(read_data(dentry.inode_index, 24, entry_point, 4) == -1){
        restore_flags(flags);
        return -1;
    }
    entry = *((uint32_t*)entry_point);

#if (TEST_EXEC == 1)
    kprintf("Part 2 Finished, entry = 0x%x, entry_point = [0x%x, 0x%x, 0x%x, 0x%x]\n", entry, entry_point[0], entry_point[1], entry_point[2], entry_point[3]);
    kprintf("    four_magics = [0x%x, 0x%x, 0x%x, 0x%x]\n", four_magics[0], four_magics[1], four_magics[2], four_magics[3]);
#endif

    //
    // Part 3 Set Paging
    //
    int32_t pcb_pos = get_and_set_new_PCB_pos();
    if(pcb_pos == -1){
        restore_flags(flags);
        return -1;
    }
    if(set_virtual_memory(pcb_pos) == -1){
        restore_flags(flags);
        return -1;
    }//set the virtual memory for the pcb

#if (TEST_EXEC == 1)
    kprintf("Part 3 Finished, pcb_pos = %d, physical_memory_start_addr = 0x%x\n", pcb_pos, get_process_memory_from_pos(pcb_pos));
#endif

    //
    // Part 4 Load EXE
    //
    if(read_data(dentry.inode_index, 0, (uint8_t*)PROGRAM_START_VIRTUAL_ADDR, PAGE_4MB_VAL) == -1){
        restore_flags(flags);
        return -1;
    }
#if (TEST_EXEC == 1)
    kprintf("Part 4 Finished\n");
#endif

    //
    // Part 5 Set PCB
    //
    PCB_t* pcb = get_pcb_from_pos(pcb_pos);
    /*assign the new pcb's value*/
    uint32_t pid = pcb_pos; // pid_alloc();
    PCB_init(pcb);
    pcb->pid = pid;
    pcb->status = 1;
    pcb->PCB_pos = pcb_pos;
    pcb->argc = argc;
    strcpy((int8_t*)pcb->argv, (int8_t*)argv);
    if(pid < MAX_NUM_OF_SCHED) pcb->parent = NULL;
    else pcb->parent = current;
    pcb->esp = 0;
    pcb->ebp = 0;
    pcb->eip = entry;
    pcb->shell_flag = 0;
    pcb->window = NULL;
    // set signal
    pcb_signal_init(pcb->PCB_pos);
    // set memory map
    pcb->physical_mem_start = get_process_memory_from_pos(pcb_pos);
    GUI_window_t* window;
    // 待修改  || strcmp(cmd, "shell")
    if(pcb->parent == NULL){
        pcb->terminal = terminal_alloc();
        if(is_modex()){
            window = GUI_window_alloc();
            GUI_window_init(window, terminal_names[terminal_get_index(pcb->terminal)], pcb->pid);
            pcb->terminal->occupied = window;
        }else{
            pcb->terminal->occupied = NULL;
        }
        pcb->terminal->ops->init(pcb->terminal);
        // pcb->terminal->ops->printf(pcb->terminal, "terminal %d\n", pcb->pid);
    }else{
        pcb->terminal = pcb->parent->terminal;
    }
    if(strcmp((const char*)cmd, "shell")){
        num_of_shell++;
        pcb->shell_flag = 1;
        // kprintf("num of shell: %d\n", num_of_shell);
    }
    // set sched info
    sched_set(pcb);
    // set top process
    top_process_set(pcb->pid);
    // kprintf("pid = %d", pid);
#if (TEST_EXEC == 1)
    kprintf("Part 5 Finished, process info: pcb_addr = 0x%x\n    pid = %d\n    status = %d\n    PCB_pos = %d\n    argc = %d\n    argv = \"%s\"\n    parent = 0x%x\n"
    , (uint32_t)pcb, pcb->pid, pcb->status, pcb->PCB_pos, pcb->argc, pcb->argv, (uint32_t)pcb->parent);
#endif

    //
    // Part 6 Set TSS
    //
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_kernel_stack_from_pos(pcb_pos);// set tss parameters
#if (TEST_EXEC == 1)
    kprintf("Part 6 Finished, tss.ss0 = 0x%x, tss.esp0 = 0x%x\n", tss.ss0, tss.esp0);
#endif

    //
    // Part 7 Save Parent Info
    //
    if(pcb->parent!=NULL){
        asm volatile(
            "movl %%esp, %0             \n\t"
            "movl %%ebp, %1             \n\t"
            "leal halt_return_addr, %2  \n\t"
            : "+r" (current->esp), "+r" (current->ebp), "+r" (current->eip)
            :
            : "cc"
        );
#if (TEST_EXEC == 1)
        kprintf("Part 7 Finished, current->esp = 0x%x, current->ebp = 0x%x\n", current->esp, current->ebp);
#endif
    }else{
#if (TEST_EXEC == 1)
        kprintf("Part 7 Finished, this is the shell, no parent\n");
#endif
    }

    //
    // Part 8 Context Switch
    //
#if (TEST_EXEC == 1)
    kprintf("Part 7 Start, SS = %d, ESP = 0x%x, CS = %d, return_address = 0x%x\n\n", (uint32_t)USER_DS, (uint32_t)USER_STACK_ADDR, (uint32_t)USER_CS, entry);
    delay();
#endif
    asm volatile(
        "pushl %0           \n\t"
        "pushl %1           \n\t"   // push user program esp
        "pushl %4           \n\t"
        "pushl %2           \n\t"   // push code segment information
        "pushl %3           \n\t"   // push user program eip
        "iret               \n\t"
        "halt_return_addr:  \n\t"
        :
        : "r" ((uint32_t)USER_DS), "r" ((uint32_t)USER_STACK_ADDR), "r" ((uint32_t)USER_CS), "r" (entry), "r" (flags)
        : "eax"
    );


    //
    // Halt Return Print
    //
    if(halt_status == EXCEPTION_HALT_STATUS) return EXCEPTION_HALT_RETVAL;  // if exception call halt, return EXCEPTION_HALT_RETVAL
    return halt_status;
}


/*  
 * sys_read
 *   DESCRIPTION: system call： read data from the keyboard, a file, RTC, and directory
 *   INPUTS: fd ：file descriptor 
 *           buf ： buffer 
 *           nbytes ： number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes if we successfully read finally
 *                 0 if we at the end of the file
 *                 -1 if fail to read
 *   SIDE EFFECTS: none
 */
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    // screen_printf(&screen,"sys_read\n");
    // return 0;
    if(fd>=FILEARR_SIZE || fd<0) return -1;
    // check if file is opened
    if(current->filearr[fd].flags == FILEDESC_FLAG_FREE) return -1;
    return current->filearr[fd].ops->read(fd, buf, nbytes);
}


/*  
 *sys_write
 *   DESCRIPTION: system call: write data to the terminal & RTC
 *   INPUTS: fd : the file descriptor 
 *           buf : buffer 
 *           nbytes : number of bytes to write
 *   OUTPUTS: none
 *   RETURN VALUE: number of bytes if we write to the terminal successfully 
 *                 0 if RTC success
 *                 -1 if failed
 *   SIDE EFFECTS: none
 */
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes){
    // screen_printf(&screen,"sys_write\n");
    // return 0;
    if(fd>=FILEARR_SIZE || fd<0) return -1;
    // check if file is opened
    if(current->filearr[fd].flags == FILEDESC_FLAG_FREE) return -1;
    return current->filearr[fd].ops->write(fd, buf, nbytes);
}


/*  
 * sys_open
 *   DESCRIPTION: system call: open a file according to its file type and return a file descriptor
 *   INPUTS: filename 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success
 *                 -1 if failed
 *   SIDE EFFECTS: change pcb parameters
 */
int32_t sys_open (const uint8_t* filename){
    // screen_printf(&screen,"sys_open\n");
    // return 0;
    if(filename==NULL) return -1;
    int32_t fd;
    dentry_t dentry;
    PCB_t* pcb = current;

    if(strncmp((int8_t*)filename, (const int8_t*)"disk0/", 6)==0){
        return ata_filesys_open(filename+5);
    }

    // find the first available file descriptor 
    for(fd=2;fd<FILEARR_SIZE;fd++){
        if(pcb->filearr[fd].flags == FILEDESC_FLAG_FREE){
            break;
        }
    }
    if(fd==FILEARR_SIZE) return -1; //if no space for new file, return -1
    if(read_dentry_by_name((int8_t*)filename, &dentry) == -1) return -1;
    filed_t* fileobj = &pcb->filearr[fd];
    fileobj->flags = FILEDESC_FLAG_INUSE;
    fileobj->file_position = 0;
    fileobj->inode_index = dentry.inode_index;  //assign the value to this file
     //check the file type
    switch(dentry.file_type){
        case 0:
            fileobj->ops = &rtc_ops;    // 0 for RTC
            break;
        case 1:
            fileobj->ops = &dir_ops;    // 1 for directory
            break;
        case 2:
            fileobj->ops = &file_ops;   // 2 for file
            break;  
        default: return -1;
    }
    if(strcmp((int8_t*)filename, (int8_t*)"window")){
        fileobj->ops = &window_ops;
    }else if (strcmp((int8_t*)filename, (int8_t*)"sound")){
        fileobj->ops = &sound_ops;
    }else if (strcmp((int8_t*)filename, (int8_t*)"mouse")){
        fileobj->ops = &mouse_ops;
    }
#if (TEST_FILE_SYSCALL == 1)
    kprintf("\nopen finished, fd = %d, filetype = %d\n\n", fd, dentry.file_type);
    delay();
#endif
    if(fileobj->ops->open(filename) == -1) return -1;
    return fd;
}

/*  
 * sys_close
 *   DESCRIPTION: system call:  close a file and release the fd
 *   INPUTS: fd 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if success
 *                 -1 if fail
 *   SIDE EFFECTS: change pcb parameters
 */
int32_t sys_close (int32_t fd){
    // screen_printf(&screen,"sys_close\n");
    // return 0;
    if(fd>=FILEARR_SIZE || fd<=1) return -1;
    // check if the file is closed before
    if(current->filearr[fd].flags == FILEDESC_FLAG_FREE) return -1;
    filed_t* fileobj = &current->filearr[fd];
    if(fileobj->ops == NULL) return -1;
    uint32_t retval = fileobj->ops->close(fd);
    //free the fd
    fileobj->flags = FILEDESC_FLAG_FREE;
    fileobj->inode_index = -1;
    fileobj->ops = NULL;
    fileobj->file_position = 0;
#if (TEST_FILE_SYSCALL == 1)
    kprintf("\nclose finished, fd = %d, return_value = %d\n\n", fd, retval);
    delay();
#endif
    return retval;
}


/*
 * getargs
 * DESCRIPTION: system call: returns the argument saved before
 *
 *  Inputs: buffer-- stores the argument
 *          nbytes: number of bytes in the buffer
 *
 *  Returns: -1: if failed
 *            0: if success
 *
 *  Side effects: return the argument to the caller
 *
 */
int32_t sys_getargs (uint8_t* buf, int32_t nbytes){
    //check if the argument valid
    if(buf==NULL || nbytes<=0 || current->argc==0) return -1; 
    uint8_t* argv = current->argv;
    int i;
    int flag = 0;
    for(i=0;i<ARGV_MAX_LEN+1;i++){
        if(argv[i]=='\0'){
            flag=1;
            i++;
            break;
        }
    }
    if(flag==0 || nbytes<i) return -1;
    // int32_t n = nbytes < ARGV_MAX_LEN ? nbytes : ARGV_MAX_LEN;
    // copy  into the buffer
    memcpy(buf, argv, i);
    return 0;
}
/*  
 * sys_vidmap
 *   DESCRIPTION: system call: map the text-mode video memory into user-space virtual address
 *   INPUTS: screen_start
 *   OUTPUTS: none
 *  Returns: -1: if failed
 *            0: if success
 *   SIDE EFFECTS: none
 */

int32_t sys_vidmap (uint8_t** screen_start){
    uint32_t flags;
    if(screen_start==NULL) return -1;
    if(!((uint32_t)screen_start>VALUE_128MB && (uint32_t)screen_start<VALUE_132MB)) return -1;
    // set page directory
    //
    cli_and_save(flags);
    terminal_t* terminal = current->terminal;
    int index = terminal_get_index(terminal);
    if (index<0 || index >2 ){
        restore_flags(flags);
        return -1;
    }
    uint32_t addr = get_addr(index);
    
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val = 0;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].present = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].rw = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].user_supervisor = 1;
    page_directory[get_dir_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val |= ((uint32_t)page_table_vga);
    // Set page table
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val = 0;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].present = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].rw = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].user_supervisor = 1;
    page_table_vga[get_pag_entry(PROGRAM_VGA_VIRTUAL_ADDR)].val |= ((uint32_t)addr);
    // reload tlb
    reload_tlb();
    *screen_start = (uint8_t*)PROGRAM_VGA_VIRTUAL_ADDR;
    restore_flags(flags);
    // screen_printf(&screen,"sys_vidmap\n");
    return 0;
}



/*  
 * sys_set_handler
 *   DESCRIPTION: system call: print "sys_set_handler"
 *   INPUTS: int32_t signum, void* handler_address
 *   OUTPUTS: "sys_set_handler"
 *   RETURN VALUE: 0 
 *   SIDE EFFECTS: none
 */
int32_t sys_set_handler(int32_t signum, void* handler_address){ 
    return user_set_signal(current->signals, signum, (signal_handler_t)handler_address);
}


/*  
 * sys_sigreturn
 *   DESCRIPTION: system call: print "sys_sigreturn"
 *   INPUTS: none
 *   OUTPUTS: "sys_sigreturn"
 *   RETURN VALUE: 0 
 *   SIDE EFFECTS: none
 */
int32_t sys_sigreturn (void){
    return user_signal_return(current->signals);
}

int32_t sys_sigsend(int32_t signum, int32_t pid){
    if(signum<0 || signum>=SIGNAL_NUM || pid<0 || pid>=MAX_PROCESS_NUM) return -1;
    return kernal_send_signal(signum, pid);
}

void* sys_malloc(uint32_t size){
    return kmalloc(size);
}

void sys_free(void* ptr){
    kfree(ptr);
    return;
}

int32_t sys_thread_create(void* func){
#if (MULTI_THREAD == 1)
    return create_thread(func);
#endif
#if (MULTI_THREAD == 0)
    return 0;
#endif
}

int32_t sys_thread_kill(int32_t pid){
#if (MULTI_THREAD == 1)
    return kill_thread(pid);
#endif
#if (MULTI_THREAD == 0)
    return 0;
#endif
}
