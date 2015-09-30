#include <mutex.h>
#include <syscall.h>
#include <atomic_xchg.h>

int mutex_init( mutex_t *mp ) {
    mp->lock = 0;
    mp->owner = -1;
}

void mutex_destroy( mutex_t *mp ){

}

void mutex_lock( mutex_t *mp ) {
    while(atomic_xchg(&mp->lock, 1) != 0){
        yield(mp->owner);
    }
    mp->owner = gettid();
}

void mutex_unlock( mutex_t *mp ){
    mp->owner = -1;
    mp->lock = 0;
}


