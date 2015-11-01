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

mutex_t lock;

void init_malloc()
{
    mutex_init(&lock);
}

/** @brief Allocates memory of size bytes
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* malloc(size_t size)
{
    mutex_lock(&lock);
    void* stuff = _malloc(size);
    mutex_unlock(&lock);
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
    mutex_lock(&lock);
    void* stuff = _memalign(alignment, size);
    mutex_unlock(&lock);
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
    mutex_lock(&lock);
    void* stuff = _calloc(nelt, eltsize);
    mutex_unlock(&lock);
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
    mutex_lock(&lock);
    void* stuff = _realloc(buf, new_size);
    mutex_unlock(&lock);
    return stuff;
}

/** @brief Frees the given memory block
 *
 *  @param buf Pointer to the memory block to be freed
 *  @return void
 **/
void free(void* buf)
{
    mutex_lock(&lock);
    _free(buf);
    mutex_unlock(&lock);
}

/** @brief Safe version of malloc
 *
 *  @param size Size of memory in bytes to be allocated
 *  @return Pointer to the allocated memory
 **/
void* smalloc(size_t size)
{
    mutex_lock(&lock);
    void* stuff = _smalloc(size);
    mutex_unlock(&lock);
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
    mutex_lock(&lock);
    void* stuff = _smemalign(alignment, size);
    mutex_unlock(&lock);
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
    mutex_lock(&lock);
    _sfree(buf, size);
    mutex_unlock(&lock);
}
