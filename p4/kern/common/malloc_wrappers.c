/** @file malloc_wrappers.c
 *  @brief Implementation of wrapper functions for the malloc family
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <mutex.h>
#include <control_block.h>
#include <assert.h>

/** @brief The mutex for all malloc related calls */
static mutex_t mutex;
/** @brief A boolean integer describing whether malloc has been initialized */
static int initialized = 0;
/** @brief A pointer to a tcb which will be freed before the next malloc call */
static tcb_t *free_later_tcb = NULL;

/** @brief Initializes state required for the thread safe malloc wrappers
 *         Before this is called the malloc functions can be called, but
 *         they will not use mutexes
 **/
void init_malloc()
{
    mutex_init(&mutex);
    initialized = 1;
}

/** @brief Acquires the lock to use malloc functions. Also, if needed frees
 *         memory which an exiting thread was unable to free by itsself
 **/
void acquire_malloc()
{
    mutex_lock(&mutex);
    if(free_later_tcb != NULL){
        finalize_exit(free_later_tcb);
        free_later_tcb = NULL;
    }
}

/** @brief Instructs malloc to free the tcb and if present, the associated ppd
 *         at a later time in another thread
 *
 *  This function is used in thread exit, and allows threads to free their
 *  tcbs and the ppd of their parent process if it is exiting without
 *  crashing, but before any other memory allocations are made
 *
 *  Must be called while the malloc mutex is held
 *
 *  @param tcb The tcb to free
 *  @return void
 **/
void free_later(tcb_t *tcb)
{
    assert(free_later_tcb == NULL);
    free_later_tcb = tcb;
}

/** @brief Release the malloc mutex
 *  @return void
 **/
void release_malloc()
{
    mutex_unlock(&mutex);
}

/** @brief Relese the malloc mutex with interrupts disabled
 *  @return void
 **/
void scheduler_release_malloc()
{
    scheduler_mutex_unlock(&mutex);
}


/** @brief Allocates memory of size bytes
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* malloc(size_t size)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _malloc(size);
        release_malloc();
    } else {
        stuff = _malloc(size);
    }
    return stuff;
}

/** @brief Allocates aligned memory of size bytes
 *
 *  @param alignment Address will be a multiple of this value
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* memalign(size_t alignment, size_t size)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _memalign(alignment, size);
        release_malloc();
    } else {
        stuff = _memalign(alignment, size);
    }
    return stuff;
}

/** @brief Allocates memory for an array of elements
 *
 *  @param nelt Number of elements in the array
 *  @param eltsize Size of each array element in bytes
 *  @return Pointer to the allocated memory
 **/
void* calloc(size_t nelt, size_t eltsize)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _calloc(nelt, eltsize);
        release_malloc();
    } else {
        stuff = _calloc(nelt, eltsize);
    }
    return stuff;
}

/** @brief Changes the size of the given memory block
 *
 *  @param buf Pointer to the existing memory block
 *  @param new_size Size of the desired new memory block
 *  @return Pointer to the allocated memory
 **/
void* realloc(void* buf, size_t new_size)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _realloc(buf, new_size);
        release_malloc();
    } else {
        stuff = _realloc(buf, new_size);
    }
    return stuff;
}

/** @brief Frees the given memory block
 *
 *  @param buf Pointer to the memory block to be freed
 *  @return void
 **/
void free(void* buf)
{
    if (initialized) {
        acquire_malloc();
        _free(buf);
        release_malloc();
    } else {
        _free(buf);
    }
}

/** @brief Safe version of malloc
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* smalloc(size_t size)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _smalloc(size);
        release_malloc();
    } else {
        stuff = _smalloc(size);
    }
    return stuff;
}

/** @brief Safe version of memalign
 *
 *  @param alignment Address will be a multiple of this value
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* smemalign(size_t alignment, size_t size)
{
    void* stuff;
    if (initialized) {
        acquire_malloc();
        stuff = _smemalign(alignment, size);
        release_malloc();
    } else {
        stuff = _smemalign(alignment, size);
    }
    return stuff;
}

/** @brief Safe version of free
 *
 *  @param buf Pointer to the memory block to be freed
 *  @param size Size of memory to be freed
 *  @return void
 **/
void sfree(void* buf, size_t size)
{
    if (initialized) {
        acquire_malloc();
        _sfree(buf, size);
        release_malloc();
    } else {
        _sfree(buf, size);
    }
}
