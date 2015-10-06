#include <syscall.h>
#include <stdlib.h>
#include <thread.h>
#include <errors.h>

static int thread_initialized = 0;

void threaded_exit()
{
    thread_initialized = 1;
}

void exit(int status)
{
    lprintf("hii");
    if (thread_initialized) {
        thr_exit((void *)status);
        EXIT_ERROR("thread exit returned");
    } else {
        set_status(status);
        vanish();
    }
}
