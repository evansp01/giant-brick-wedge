/** @file mutex.c
 *  @brief An implementation of mutexes
 *
 *  NOTE: could use fetch and add for ticket locks to ensure real fairness
 *
 *  @author Evan Palmer (esp)
 **/
#include <mutex.h>
#include <thread.h>
#include <thr_internals.h>
#include <simics.h>

inline void yield_owner_or_other(int tid)
{
    if (thr_yield(tid) < 0) {
        thr_yield(-1);
    }
}

int mutex_init(mutex_t* mp)
{
    //unlocked and nobody owns
    mp->lock = 0;
    mp->waiting = 0;
    mp->owner = -1;
    return 0;
}

void mutex_destroy(mutex_t* mp)
{
    //locked but nobody owns
    mp->lock = 1;
    mp->owner = -1;
}

void mutex_lock(mutex_t* mp)
{
    //might as well get the tid, we'll need it later
    int thread_id = thr_getid();
    //let's see if we can get the lock immediately
    if (atomic_xchg(&mp->lock, 1) == 0) {
        mp->owner = thread_id;
        return;
    }
    //we failed to get the lock, note that we are waiting
    atomic_inc(&mp->waiting);
    //now try and get the lock again
    do {
        //yield to whoever owns the lock
        yield_owner_or_other(mp->owner);
    }
    while (atomic_xchg(&mp->lock, 1) != 0);
    //we got  the lock, stop waiting
    atomic_dec(&mp->waiting);
    mp->owner = thread_id;
}

void mutex_unlock(mutex_t* mp)
{
    if (mp->lock == 1 && mp->owner == -1) {
        lprintf("cannot unlock destroyed mutex\n");
        return;
    }
    //if people are waiting, we will yield so we don't get the lock too much
    if (mp->waiting > 0) {
        mp->owner = -1;
        mp->lock = 0;
        thr_yield(-1);
    } else {
        mp->owner = -1;
        mp->lock = 0;
    }
}
