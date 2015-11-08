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

#define LOCKED 1
#define UNLOCKED 0
#define UNSPECIFIED -1

/** @brief Initialize mutex
 *  This initializes a mutex allowing it to be locked. Mutexes are initialized
 *  to the unlocked state.
 *
 *  @param mp The mutex to initialize
 *  @return zero on success, less than zero on failure
 **/
int mutex_init(mutex_t* mp)
{
    //unlocked and nobody owns
    mp->lock = UNLOCKED;
    mp->owner = UNSPECIFIED;
    mp->waiting = 0;
    return 0;
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
    //lock the mutex
    if (atomic_xchg(&mp->lock, LOCKED) == LOCKED) {
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
    //might as well get the tid, we'll need it later
    tcb_t *tcb = get_tcb();
    int thread_id = tcb->id;
    //let's see if we can get the lock immediately (provided nobody is waiting)
    if (atomic_xchg(&mp->lock, LOCKED) == UNLOCKED) {
        mp->owner = thread_id;
        return;
    }
    //we failed to get the lock, note that we are waiting
    atomic_inc(&mp->waiting);
    //now try and get the lock again
    do {
        // try to yield to the owner
        if (yield(mp->owner) < 0) {
            //but if that doesn't work, settle for anyone
            yield(UNSPECIFIED);
        }
    } while (atomic_xchg(&mp->lock, LOCKED) != UNLOCKED);
    //we got  the lock, stop waiting
    atomic_dec(&mp->waiting);
    mp->owner = thread_id;
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
    if (mp->lock == LOCKED && mp->owner == UNSPECIFIED) {
        panic("cannot unlock kernel mutex which is destroyed or not owned");
    }
    //if people are waiting, we will yield so we don't get the lock too much
    if (mp->waiting > 0) {
        mp->owner = UNSPECIFIED;
        mp->lock = UNLOCKED;
        yield(UNSPECIFIED);
    } else {
        mp->owner = UNSPECIFIED;
        mp->lock = UNLOCKED;
    }
}
