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

static int malloc_initialized = 0;
static mutex_t malloc_mutex;

inline void ensure_initialized()
{
    if (!malloc_initialized) {
        mutex_init(&malloc_mutex);
    }
}

void* malloc(size_t __size)
{
    ensure_initialized();
    mutex_lock(&malloc_mutex);
    void* return_val = _malloc(__size);
    mutex_unlock(&malloc_mutex);
    return return_val;
}

void* calloc(size_t __nelt, size_t __eltsize)
{
    ensure_initialized();
    mutex_lock(&malloc_mutex);
    void* return_val = _calloc(__nelt, __eltsize);
    mutex_unlock(&malloc_mutex);
    return return_val;
}

void* realloc(void* __buf, size_t __new_size)
{
    ensure_initialized();
    mutex_lock(&malloc_mutex);
    void* return_val = _realloc(__buf, __new_size);
    mutex_unlock(&malloc_mutex);
    return return_val;
}

void free(void* __buf)
{
    ensure_initialized();
    mutex_lock(&malloc_mutex);
    free(__buf);
    mutex_unlock(&malloc_mutex);
}
