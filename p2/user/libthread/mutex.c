#include <mutex.h>
#include <syscall.h>
#include <thr_internals.h>
#include <simics.h>

int mutex_init( mutex_t *mp ) {
    //unlocked and nobody owns
    mp->lock = 0;
    mp->owner = -1;
    return 0;
}

void mutex_destroy( mutex_t *mp ){
    //locked but nobody owns
    mp->lock = 1;
    mp->owner = -1;
}

void mutex_lock( mutex_t *mp ) {
    while(atomic_xchg(&mp->lock, 1) != 0){
        yield(mp->owner);
    }
    mp->owner = gettid();
}

void mutex_unlock( mutex_t *mp ){
    if(mp->lock == 1 && mp->owner == -1){
        lprintf("cannot unlock destroyed mutex\n");
        return;
    }
    mp->owner = -1;
    mp->lock = 0;
}


