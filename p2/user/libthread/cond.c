#include <cond.h>
#include <thr_internals.h>
#include <syscall.h>

int cond_init( cond_t *cv ) {
    mutex_init(&cv->cv_mutex);
    QUEUE_INIT(&cv->cv_queue);
    return 0;
}
void cond_destroy( cond_t *cv ){
    mutex_destroy(&cv->cv_mutex);

}
void cond_wait( cond_t *cv, mutex_t *mp ){

}
void cond_signal( cond_t *cv ){

}
void cond_broadcast( cond_t *cv ){

}
