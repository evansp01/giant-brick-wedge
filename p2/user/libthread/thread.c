/** @file thread.c
 *  @brief This file defines thread-management interface.
 *
 *  It should NOT contain any of the thread library internals.
 *  Therefore, you may NOT modify this file.
 *
 *  However, you MAY modify user/libthread/thr_internals.h.
 *
 */
#include <thr_internals.h>
#include <stdlib.h>
#include <autostack.h>
#include <syscall.h>
#include <thread.h>
#include <simics.h>

/* thread library functions */
int thr_init(unsigned int size)
{
    install_threaded();
    void *stack_low, *stack_high;
    get_stack_bounds(&stack_high, &stack_low);
    frame_alloc_init(size, stack_high, stack_low);
    MAGIC_BREAK;
    //something about malloc?
    //mutexes??????
    return 0;
}

void thr_wrapper(void *(*func)(void*), void *arg, int *stack_base){
    *stack_base = gettid();
    //hey it would be cool to have a tcb entry
    void *status = func(arg);
    thr_exit(status);
}

int thr_join(int tid, void** statusp)
{
    return 0;
}

void thr_exit(void* status)
{
    //put exit status in tcb
    //tcb_add_exit(gettid(), status);
    //get my stack pointer
    //free_and_vanish
    void *stack = NULL;
    free_frame_and_vanish(stack);
}

int thr_getid(void)
{
    return gettid();
}

int thr_yield(int tid)
{
    return yield(tid);
}
