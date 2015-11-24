/*
 * these functions should be thread safe.
 * It is up to you to rewrite them
 * to make them thread safe.
 *
 */
#include <stdlib.h>
#include <types.h>
#include <mutex.h>
#include <stddef.h>
#include <malloc.h>
#include <thr_internals.h>

static int thread_initialized = 0;
static mutex_t malloc_mutex;

void initialize_malloc()
{
    thread_initialized = 1;
    mutex_init(&malloc_mutex);
}

void* malloc(size_t __size)
{
    if (thread_initialized) {
        mutex_lock(&malloc_mutex);
        void* return_val = _malloc(__size);
        mutex_unlock(&malloc_mutex);
        return return_val;
    } else {
        return _malloc(__size);
    }
}

void* calloc(size_t __nelt, size_t __eltsize)
{
    if (thread_initialized) {
        mutex_lock(&malloc_mutex);
        void* return_val = _calloc(__nelt, __eltsize);
        mutex_unlock(&malloc_mutex);
        return return_val;
    } else {
        return _calloc(__nelt, __eltsize);
    }
}

void* realloc(void* __buf, size_t __new_size)
{
    if (thread_initialized) {
        mutex_lock(&malloc_mutex);
        void* return_val = _realloc(__buf, __new_size);
        mutex_unlock(&malloc_mutex);
        return return_val;
    } else {
        return _realloc(__buf, __new_size);
    }
}

void free(void* __buf)
{
    if (thread_initialized) {
        mutex_lock(&malloc_mutex);
        _free(__buf);
        mutex_unlock(&malloc_mutex);
    } else {
        _free(__buf);
    }
}
