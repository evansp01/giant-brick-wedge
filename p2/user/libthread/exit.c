#include <syscall.h>
#include <stdlib.h>
#include <thread.h>
#include <errors.h>

static int thread_initialized = 0;

/** @brief Instruct exit to call thr_exit instead of exiting normally
 *  @return void
 **/
void threaded_exit()
{
    thread_initialized = 1;
}

/** @brief Links over the default exit method, and allows for threaded exit
 *  This exit function will call thr_exit instead of set_status and vanish
 *  if threaded_exit has been called.
 *
 *  @param status The exit status
 *  @return Does not return
 **/
void exit(int status)
{
    if (thread_initialized) {
        thr_exit((void *)status);
        EXIT_ERROR("thread exit returned");
    } else {
        set_status(status);
        vanish();
    }
}
