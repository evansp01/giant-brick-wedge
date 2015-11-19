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
#define DESTROYED 2

static int initialized = 0;

/** @brief Simple lock to prevent threads from interleaving
 *  In the current single processor kernel implementation, this only involves
 *  disabling interrupts.
 *  
 *  @return void
 **/
void lock()
{
    disable_interrupts();
}

/** @brief Simple unlock to allow threads to resume execution
 *  In the current single processor kernel implementation, this only involves
 *  re-enabling interrupts.
 *  
 *  @return void
 **/
void unlock()
{
    enable_interrupts();
}

/** @brief Initializes all mutexes for multithreaded usage
 *  This function enables mutexes once the kernel has been fully initialized.
 *  
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
    mp->owner = UNSPECIFIED;
    mp->count = 1;
    Q_INIT_HEAD(&mp->waiting);
}

/** @brief Destroy a mutex
 *  Mutexes cannot be operated on after being destroyed unless init is called
 *  again. It is illegal to destroy a locked mutex.
 *
 *  @param mp The mutex to destroy
 *  @return void
 **/
void mutex_destroy(mutex_t* mp)
{
    if (mp->count < 1) {
        panic("kernel mutex destroyed while holding lock");
    }
    // Mark the mutex as destroyed
    mp->count = DESTROYED;
}

/** @brief Lock a mutex
 *  Blocks until the lock for this mutex is acquired.
 *
 *  @param mp The mutex to lock
 *  @return void
 **/
void mutex_lock(mutex_t* mp)
{
    if (initialized) {
        if (mp->count >= DESTROYED) {
            panic("cannot lock kernel mutex which is destroyed");
        }
        tcb_t *tcb = get_tcb();
        lock();
        mp->count--;
        if (mp->count < 0) {
            Q_INSERT_TAIL(&mp->waiting, tcb, suspended_threads);
            deschedule(tcb);
        }
        unlock();
        mp->owner = tcb->id;
    }
}

/** @brief Unlock a mutex
 *  Unlocks the given mutex. It is illegal for a thread other than the thread
 *  which called lock to unlock a mutex.
 *
 *  @param mp The mutex to unlock
 *  @return void
 **/
void mutex_unlock(mutex_t* mp)
{
    if (initialized) {
        // TODO: Replace once finalize_exit has been moved
        //tcb_t *tcb = get_tcb();
        //if (mp->count >= DESTROYED || mp->owner != tcb->id) {
        if (mp->count >= 1 || mp->owner == UNSPECIFIED) {
            lprintf("count %d, owner %d", mp->count, mp->owner);
            panic("cannot lock kernel mutex which is destroyed or not owned");
        }
        lock();
        mp->owner = UNSPECIFIED;
        mp->count++;
        // wake the next thread up
        if (!Q_IS_EMPTY(&mp->waiting)) {
            tcb_t *tcb_to_schedule = Q_GET_FRONT(&mp->waiting);
            Q_REMOVE(&mp->waiting, tcb_to_schedule, suspended_threads);
            schedule(tcb_to_schedule);
        }
        unlock();
    }
}

/** @brief Unlock a mutex
 *  Unlocks the given mutex without re-enabling interrupts. It is illegal for
 *  a thread other than the thread which called lock to unlock a mutex.
 *
 *  @param mp The mutex to unlock
 *  @return void
 **/
void scheduler_mutex_unlock(mutex_t* mp)
{
    if (initialized) {
        tcb_t *tcb = get_tcb();
        if (mp->count >= DESTROYED || mp->owner != tcb->id) {
            panic("cannot lock kernel mutex which is destroyed or not owned");
        }
        mp->owner = UNSPECIFIED;
        mp->count++;
        // wake the next thread up
        if (!Q_IS_EMPTY(&mp->waiting)) {
            tcb_t *tcb_to_schedule = Q_GET_FRONT(&mp->waiting);
            Q_REMOVE(&mp->waiting, tcb_to_schedule, suspended_threads);
            schedule_interrupts_disabled(tcb_to_schedule);
        }
    }
}
