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
#include <sem.h>

static sem_t sem;
static int initialized = 0;

void init_malloc()
{
    sem_init(&sem, 1);
    initialized = 1;
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
        sem_wait(&sem);
        stuff = _malloc(size);
        sem_signal(&sem);
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
        sem_wait(&sem);
        stuff = _memalign(alignment, size);
        sem_signal(&sem);
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
        sem_wait(&sem);
        stuff = _calloc(nelt, eltsize);
        sem_signal(&sem);
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
        sem_wait(&sem);
        stuff = _realloc(buf, new_size);
        sem_signal(&sem);
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
        sem_wait(&sem);
        _free(buf);
        sem_signal(&sem);
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
        sem_wait(&sem);
        stuff = _smalloc(size);
        sem_signal(&sem);
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
        sem_wait(&sem);
        stuff = _smemalign(alignment, size);
        sem_signal(&sem);
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
        sem_wait(&sem);
        _sfree(buf, size);
        sem_signal(&sem);
    } else {
        _sfree(buf, size);
    }
}
