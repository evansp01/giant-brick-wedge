/** @file mutex.c
 *  @brief An implementation of mutexes
 *
 *  NOTE: could use fetch and add for ticket locks to ensure real fairness
 *
 *  @author Evan Palmer (esp)
 **/
#include <mutex.h>
#include <thread.h>
#include <syscall.h>
#include <thr_internals.h>
#include <errors.h>


#define LOCKED 1
#define UNLOCKED 0
#define UNSPECIFIED -1

int mutex_init(mutex_t* mp)
{
    //unlocked and nobody owns
    mp->lock = LOCKED;
    mp->owner = UNSPECIFIED;
    mp->waiting = 0;
    return 0;
}

void mutex_destroy(mutex_t* mp)
{
    //lock the mutex
    if (atomic_xchg(&mp->lock, LOCKED) == LOCKED) {
        EXIT_ERROR("mutex destroyed while holding lock");
    }
    //set the owner to nobody
    mp->owner = UNSPECIFIED;
}

void mutex_lock(mutex_t* mp)
{
    //might as well get the tid, we'll need it later
    int thread_id = thr_getid();
    //let's see if we can get the lock immediately
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

void mutex_unlock(mutex_t* mp)
{
    if (mp->lock == LOCKED && mp->owner == UNSPECIFIED) {
        WARN("cannot unlock destroyed mutex");
        return;
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
