#include <cond.h>
#include <thr_internals.h>
#include <syscall.h>
#include <thread.h>

int cond_init(cond_t* cv)
{
    if (mutex_init(&cv->m) < 0) {
        return -1;
    }
    QUEUE_INIT(&cv->waiting);
    return 0;
}

void cond_destroy(cond_t* cv)
{
    mutex_destroy(&cv->m);
    QUEUE_FREE(&cv->waiting);
}

void cond_wait(cond_t* cv, mutex_t* mp)
{
    int dontreject = 0;
    mutex_lock(&cv->m);
    QUEUE_ADD(&cv->waiting, thr_getid());
    mutex_unlock(mp);
    //atomically
    mutex_unlock(&cv->m);
    deschedule(&dontreject);
    //end atomically
    mutex_lock(&cv->m);
}

void cond_signal(cond_t* cv)
{
    int tid;
    mutex_lock(&cv->m);
    // if nobody is waiting, just return
    if (!QUEUE_EMPTY(&cv->waiting)) {
        tid = QUEUE_REMOVE(&cv->waiting);
        while (make_runnable(tid) < 0) {
            //the other guy must not have fallen asleep yet, let's encourage
            yield(tid);
        }
    }
    mutex_unlock(&cv->m);
}

void cond_broadcast(cond_t* cv)
{
    int tid;
    mutex_lock(&cv->m);
    // if nobody is waiting, just return
    while (!QUEUE_EMPTY(&cv->waiting)) {
        tid = QUEUE_REMOVE(&cv->waiting);
        while (make_runnable(tid) < 0) {
            //the other guy must not have fallen asleep yet, let's encourage
            yield(tid);
        }
    }
    mutex_unlock(&cv->m);
}
