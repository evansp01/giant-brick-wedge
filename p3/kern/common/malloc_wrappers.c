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

int threaded = 0;
sem_t sem;

void init_malloc()
{
    threaded = 1;
    sem_init(&sem, 1);
}

/** @brief Allocates memory of size bytes
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* malloc(size_t size)
{
    if (!threaded) {
        return _malloc(size);
    }
    sem_wait(&sem);
    void* stuff = _malloc(size);
    sem_signal(&sem);
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
    if (!threaded) {
        return _memalign(alignment, size);
    }
    sem_wait(&sem);
    void* stuff = _memalign(alignment, size);
    sem_signal(&sem);
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
    if (!threaded) {
        return _calloc(nelt, eltsize);
    }
    sem_wait(&sem);
    void* stuff = _calloc(nelt, eltsize);
    sem_signal(&sem);
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
    if (!threaded) {
        return _realloc(buf, new_size);
    }
    sem_wait(&sem);
    void* stuff = _realloc(buf, new_size);
    sem_signal(&sem);
    return stuff;
}

/** @brief Frees the given memory block
 *
 *  @param buf Pointer to the memory block to be freed
 *  @return void
 **/
void free(void* buf)
{
    if (!threaded) {
        _free(buf);
        return;
    }
    sem_wait(&sem);
    _free(buf);
    sem_signal(&sem);
}

/** @brief Safe version of malloc
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* smalloc(size_t size)
{
    if (!threaded) {
        return _smalloc(size);
    }
    sem_wait(&sem);
    void* stuff = _smalloc(size);
    sem_signal(&sem);
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
    if (!threaded) {
        return _smemalign(alignment, size);
    }
    sem_wait(&sem);
    void* stuff = _smemalign(alignment, size);
    sem_signal(&sem);
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
    if (!threaded) {
        _sfree(buf, size);
        return;
    }
    sem_wait(&sem);
    _sfree(buf, size);
    sem_signal(&sem);
}
