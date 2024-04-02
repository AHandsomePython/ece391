#include "spinlock.h"

/* 
 *   spin_lock
 *   DESCRIPTION: This is spin lock function. It will continuously try to acquire a lock 
 *   INPUTS: lock---1 is locked and 0 is unlocked
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: get spin lock
 */
inline void spin_lock(spinlock_t* lock){
    asm volatile(
        "movl %0, %%eax             \n\t"
        "0: xchgl (%1), %%eax       \n\t" // try to acquire lock
        "cmpl  %0 ,%%eax            \n\t"
        "je 0b                      \n\t" // if fails, spin
        :
        :"i"(SPIN_LOCK_LOCKED),  "r"(lock) // 2 inputs 
        :"eax","memory","cc" // clobbed list
    );
}

/* 
 *   spin_unlock
 *   DESCRIPTION: This is spin lock function. It will unlock the lock
 *   INPUTS: lock---1 is locked and 0 is unlocked
 *   OUTPUTS: none 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: unlock spin lock
 */
inline void spin_unlock(spinlock_t* lock){
    *(lock) = SPIN_LOCK_UNLOCKED;
}
