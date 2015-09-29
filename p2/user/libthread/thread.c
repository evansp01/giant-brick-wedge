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
#include <autostack.h>

/* thread library functions */
int thr_init(unsigned int size)
{
    install_threaded();
    void *stack_low, *stack_high;
    get_stack_bounds(&stack_high, &stack_low);
    frame_alloc_init(size, stack_high, stack_low);
    //something about malloc?
    //mutexes??????
    return 0;
}
int thr_create(void* (*func)(void*), void* args)
{
    return 0;
}
int thr_join(int tid, void** statusp)
{
    return 0;
}
void thr_exit(void* status)
{
}
int thr_getid(void)
{
    return 0;
}
int thr_yield(int tid)
{
    return 0;
}
