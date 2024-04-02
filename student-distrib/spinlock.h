#ifndef SPINLOCK_H
#define SPINLOCK_H



#define SPIN_LOCK_LOCKED 1
#define SPIN_LOCK_UNLOCKED 0

/* 
 *   spin_lock_irqsave
 *   DESCRIPTION: This is spin lock function. It will store flag, clear INTR flag and continuously try to acquire a lock 
 *   INPUTS: lock---1 is locked and 0 is unlocked
 *           flags--- store the intp flag 
 *   OUTPUTS: output to port 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: store flag, clear interrupt, try to get spin lock
 */

#define spin_lock_irqsave(lock, flags)      \
do{                                         \
    asm volatile(                           \
        "pushfl             \n\t"           \
        "popl %0            \n\t"           \
        "cli                \n\t"           \
        : "+r" (flags)                      \
        :                                   \
        : "cc"                              \
    );                                      \
    spin_lock(lock);                        \
} while(0)


/* 
 *   spin_lock_irqrestore
 *   DESCRIPTION: This is spin lock function. It will release the lock, and restore INTP flag from variable
 *   INPUTS: lock---1 is locked and 0 is unlocked
 *           flags---used to restore flag 
 *   OUTPUTS: output to port 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: restore flag, release spin lock
 */

#define spin_unlock_irqrestore(lock, flags) \
do {                                        \
    spin_unlock(lock);                      \
    asm volatile (                          \
            "pushl %0       \n\t"           \
            "popfl          \n\t"           \
            :                               \
            : "r"(flags)                    \
            : "cc"                          \
    );                                      \
} while (0)

typedef unsigned int spinlock_t;

inline void spin_lock(spinlock_t* lock);

inline void spin_unlock(spinlock_t* lock);

#endif /* SPINLOCK_H */
