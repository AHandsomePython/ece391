# Extra Credit

## Part 1. GUI
### 1. ModeX Screen Support (GUI/screen.c, GUI/gui_screen.c)
-  **Automatic double buffer** management for image buffering **without explicit operation**

-  **Unified** screen operation interface  



### 2. Window Support (GUI/window.c, GUI/gui.c)
- **Virtualization** of the window. The user state needn't know any information about the size and position of the window, and from the user's view, **each process seems to have the entire screen**  

- **Multiple windows support** (on the same screen).  Supports displaying any number of windows simultaneously  

-  Management and display of **multi-window overlapping** relationships (multi-layer management)  

- Support for **screen pre-rendering** to save computational resources (dynamically buffers the underlying window image)  

- Window support for **mouse-based movement, zooming, switching, full screen, minimizing**  

- Support mouse-based **front-end scheduling**. Window scheduling can cooperate with the scheduler for front-end process switching  

- Provides a **flexible system call interface** for users to draw windows   



### 3. GUI Terminal Support (GUI/gui_terminal.c)
- Through the unified terminal function predefined interface, we can **quickly switch between text terminal and GUI terminal** without modifying other code   

- **Each terminal has its own window**, and like other types of window applications, terminal also supports the operations supported by the window, as well as mouse-based pre-stage scheduling

- **GUI and text terminal share the same set of system call interfaces**, easy to maintain and extend



### 4. Widget Support (GUI/gui_interaction.c)
- Provides a **general interface** to the widget

- Support **automatic capture and processing of widget activities**   

- Support **multi-level widget nesting** 

- **System-level menus** are implemented based on widgets (including the start menu, palette interface, color picker, right-click menu, etc.) 



### 5. Dynamic Color Settings and Palette Settings (GUI/gui_color.c, GUI/gui_interaction.c)

-  Support **dynamic color palette** change  

-  Support **dynamic adjustment** of **system component colors**



### 6. User interaction support (GUI/gui.c, GUI/gui_interaction.c)
-  Support mouse-based **graphical interaction**, including **movement**, **widget triggering**, **swiping**, **taskbar**, etc.



## Part 2. Memory Management
### 1. Overall Structure
- The system's memory management implements a **three-level structure**. The physical memory management is based on **Buddy System** to **reduce ** **external fragmentation**. The **Slab Cache**, an object-based allocation cache, is built on top of the Buddy System for **fast allocation and release** of  objects of the same size. A **kmalloc** **allocator** is built on the Slab layer to **allocate memory of arbitrary size**.



### 2. Buddy system-based physical memory management  (memoryalloc/buddy.c)

- **Reduce external fragmentation** by using the Buddy System for physical memory management. The Buddy System also implements a **contiguous page allocator.**



### 3. Slab Cache (memoryalloc/slab.c)
- Using **Slab Cache** for object management and creating a buffer pool for fixed-length sizes can **greatly reduce the number of page mappings**



### 4. kmalloc (memoryalloc/malloc.c)
- **kmalloc allocator** is created on Slab Cache **for small and medium-sized objects**



### 5. User malloc (memoryalloc/malloc.c)
-  Build on the above three systems to **implement user-state memory allocation**



## Part 3. File System
### 1.  ATA Disk Driver (disk/ata.c)

- Support **ATA disk block sequential read/write**

- Support to **obtain ATA information**, including spatial information, etc.  



### 2. ATA Disk-Based Permanent Read/Write File System (disk/disk_filesys.c, syscall.c, filesys.c) 

- **Multi-level directory support** and support for using the **parent directory** identifier "..." in the path and the **current directory** identifier "." 

-  **Dynamic inode, datablock allocation**. Support for **bitmap** to provide fast finding of free blocks 

-  Support **dynamic creation of files and directories**  

-  Support **file writing** and dynamic expansion of data blocks  

-  Support **superblock write-back to update file information**   

-  Support for **disk formatting** 

-  Support **Unified File Interface (VFS)**, coexisting and **compatible with the original** memory file system    

-  This file system is mounted in a directory of the original memory file system 



## Part 4. Signal
### 1. Support all Signal Functions Requested by MP3 Doc (signal.c)

- Support **user-defined Signal** behavior

- Support **RTC to send ALARM signal** at regular intervals

- Support keyboard to send **INTERRUPT signal** via **Ctrl+C** 



### 2. Support User State Sending Signal to Other Processes(signal.c)

-  Support this operation to **facilitate communication between multi-threaded programs**




## Part 5. Multi-Thread Support
### 1. Multi-Thread Support (scheduling.c)  

-  Support **user-state** processes to **create threads** based on functions

-  **Threads inherit the address space of the process and have a separate user stack**

-  Support for running **multi-threaded programs** in user state (each process can create >= 1 thread) 

-  Support **address space sharing** and **variable sharing**  

-  Support user state process kill a thread 

-  Threads also support Signal. Customizable Signal behavior for **thread communication and synchronization** 



### 2. Sleep Support (scheduling.c)

-  **Sleep and wake-up** support, tell the scheduler to hibernate and wake up through flag bit setting




## Part 6. Devices
### 1. Mouse (GUI/mouse.c)
-  **Virtualization**. The mouse returns to the user the position inside the window. The movement of the window does not on the user state program



### 2. Sound (device/sound.c)

-  Support for making **individual note sounds**



### 3. RTC (rtc.c)

-  **Random number** generator
