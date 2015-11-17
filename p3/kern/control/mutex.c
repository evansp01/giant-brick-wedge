/** @file mutex.c
 *  @brief Implementation of functions for locking
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/
 
#include <mutex.h>
#include <atomic.h>
#include <stdlib.h>
#include <control.h>
#include <scheduler.h>
#include <asm.h>

#include <simics.h>

#define UNSPECIFIED -1

static int initialized = 0;

/** @brief Initializes all mutexes for multithreaded usage
 *  @return void
 **/
void init_mutexes()
{
    initialized = 1;
}

/** @brief Initialize mutex
 *  This initializes a mutex allowing it to be locked. Mutexes are initialized
 *  to the unlocked state.
 *
 *  @param mp The mutex to initialize
 *  @return zero on success, less than zero on failure
 **/
void mutex_init(mutex_t* mp)
{
    //unlocked and nobody owns
    mp->owner = UNSPECIFIED;
    mp->count = 1;
    Q_INIT_HEAD(&mp->waiting);
}

/** @brief Destroy a mutex
 *  Mutexes cannot be operated on after being destroyed unless init is called
 *  again. It is illegal to destroy a locked mutex
 *
 *  @param mp The mutex to destroy
 *  @return void
 **/
void mutex_destroy(mutex_t* mp)
{
    if (mp->count < 1) {
        panic("kernel mutex destroyed while holding lock");
    }
    //set the owner to nobody
    mp->owner = UNSPECIFIED;
}

/** @brief Lock a mutex
 *  Blocks until the lock for this mutex is acquired
 *
 *  @param mp The mutex to lock
 *  @return void
 **/
void mutex_lock(mutex_t* mp)
{
    if (initialized) {
        tcb_t *tcb = get_tcb();
    
        disable_interrupts();
        mp->count--;
        if (mp->count < 0) {
            Q_INSERT_TAIL(&mp->waiting, tcb, suspended_threads);
            //lprintf("mutex lock descheduling %d for mutex 0x%x, count is %d", tcb->id, (int)mp, mp->count);
            deschedule(tcb);
        }
        else {
            //lprintf("mutex lock with count 1 for thread %d for mutex 0x%x, count is %d", tcb->id, (int)mp, mp->count);
        }
        enable_interrupts();
        mp->owner = tcb->id;
    }
}

/** @brief Unlock a mutex
 *  Unlocks the given mutex. It is illegal for a thread other than the thread
 *  which called lock to unlock a mutex
 *
 *  @param mp The mutex to unlock
 *  @return void
 **/
void mutex_unlock(mutex_t* mp)
{
    if (initialized) {
        if (mp->count == 1 && mp->owner == UNSPECIFIED) {
            panic("cannot unlock kernel mutex which is destroyed or not owned");
        }
        
        disable_interrupts();
        
        mp->owner = UNSPECIFIED;
        mp->count++;
        
        // wake the next thread up
        if (!Q_IS_EMPTY(&mp->waiting)) {
            tcb_t *tcb_to_schedule = Q_GET_FRONT(&mp->waiting);
            Q_REMOVE(&mp->waiting, tcb_to_schedule, suspended_threads);
            schedule_interrupts_disabled(tcb_to_schedule);
            //lprintf("mutex unlock scheduled %d for mutex 0x%x, count is %d", tcb_to_schedule->id, (int)mp, mp->count);
        }
        else {
            //lprintf("mutex unlock but no waiting threads for mutex 0x%x, count is %d", (int)mp, mp->count);
        }
        
        enable_interrupts();
    }
}


/** @brief Unlock a mutex
 *  Unlocks the given mutex. It is illegal for a thread other than the thread
 *  which called lock to unlock a mutex
 *
 *  @param mp The mutex to unlock
 *  @return void
 **/
void scheduler_mutex_unlock(mutex_t* mp)
{
    if (initialized) {
        if (mp->count == 1 && mp->owner == UNSPECIFIED) {
            panic("cannot unlock kernel mutex which is destroyed or not owned");
        }
        
        mp->owner = UNSPECIFIED;
        mp->count++;
        
        // wake the next thread up
        if (!Q_IS_EMPTY(&mp->waiting)) {
            tcb_t *tcb_to_schedule = Q_GET_FRONT(&mp->waiting);
            Q_REMOVE(&mp->waiting, tcb_to_schedule, suspended_threads);
            schedule_interrupts_disabled(tcb_to_schedule);
            //lprintf("scheduler mutex unlock scheduled %d for mutex 0x%x, count is %d", tcb_to_schedule->id, (int)mp, mp->count);
        }
        else {
            //lprintf("scheduler mutex unlock but no waiting threads for mutex 0x%x, count is %d", (int)mp, mp->count);
        }
    }
}
