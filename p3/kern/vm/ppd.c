/** @file ppd.c
 *
 *  @brief Functions to handle page directories
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <vm.h>
#include <mutex.h>
#include <cr.h>
#include "vm_internal.h"
#include <stdlib.h>
#include <malloc.h>
#include <control_block.h>
#include <asm.h>
#include <malloc_internal.h>
#include <assert.h>
#include <malloc_wrappers.h>

/** @brief Initialize a process page directory
 *
 *  @return The new ppd or NULL on failure
 **/
ppd_t* init_ppd()
{
    ppd_t* ppd = malloc(sizeof(ppd_t));
    if (ppd == NULL) {
        return NULL;
    }
    if(H_INIT_TABLE(&ppd->alloc_table) < 0){
        sfree(ppd, sizeof(ppd_t));
        return NULL;
    }
    mutex_init(&ppd->lock);
    if ((ppd->dir = alloc_page_directory()) == NULL) {
        mutex_destroy(&ppd->lock);
        sfree(ppd, sizeof(ppd_t));
        return NULL;
    }
    return ppd;
}

/** @brief Copy a user allocation
 *
 *  @param to The allocation to copy to
 *  @param from The allocation to copy from
 *  @return void
 **/
void copy_alloc(alloc_t* to, alloc_t* from)
{
    to->start = from->start;
    to->size = from->size;
}

/** @brief Free the memory associated with an alloc_t struct
 *
 *  @param alloc The pointer to free
 *  @return void
 **/
void free_alloc(alloc_t* alloc)
{
    sfree(alloc, sizeof(alloc_t));
}

/** @brief Record an allocation to this ppd
 *
 *  @param ppd The ppd allocated to
 *  @param start The start of the allocation
 *  @param size The size of the allocation
 *  @return Zero on success, less than zero on failure
 **/
int add_alloc(ppd_t* ppd, void* start, uint32_t size)
{
    alloc_t* new_alloc = (alloc_t*)smalloc(sizeof(alloc_t));
    if (new_alloc == NULL) {
        return -1;
    }
    Q_INIT_ELEM(new_alloc, list);
    new_alloc->start = (uint32_t)start;
    new_alloc->size = size;
    H_INSERT(&ppd->alloc_table, new_alloc, start, list);
    return 0;
}

/** @brief Free an allocation associated with this page directory
 *
 *  @param ppd The ppd allocated to
 *  @param start The start of the allocation
 *  @return Zero on success, less than zero on failure
 **/
int vm_free(ppd_t* ppd, void* start)
{
    alloc_t* alloc = H_REMOVE(&ppd->alloc_table, (uint32_t)start, start, list);
    if(alloc == NULL){
        return -1;
    }
    vm_free_alloc(ppd, alloc->start, alloc->size);
    free_alloc(alloc);
    return 0;
}

/** @brief Copies the list of allocations from one process to another
 *
 *  @param to The ppd to copy the list to
 *  @param from The ppd to copy the list from
 *  @returns Zero on success, less than 0 on failure
 **/
int copy_alloc_list(ppd_t* to, ppd_t* from)
{
    int i;
    alloc_t* alloc;
    int success = 1;
    H_FOREACH(i, alloc, &from->alloc_table, list)
    {
        alloc_t* copy = malloc(sizeof(alloc_t));
        Q_INIT_ELEM(copy, list);
        if (copy == NULL) {
            success = 0;
            break;
        }
        copy_alloc(copy, alloc);
        H_INSERT(&to->alloc_table, copy, start, list);
    }
    if (success) {
        return 0;
    }
    // free the hash table memory
    alloc_t* swap;
    H_FOREACH_SAFE(i, alloc, swap, &to->alloc_table, list)
    {
        free_alloc(alloc);
    }
    H_FREE_TABLE(&to->alloc_table);
    return -1;
}



/** @brief Free all user memory allocations associated with this ppd
 *
 *  @param to_free The ppd to free allocations from
 *  @return void
 **/
void free_ppd_user_mem(ppd_t* to_free)
{
    int i;
    alloc_t* alloc;
    alloc_t* swap;
    H_FOREACH_SAFE(i, alloc, swap, &to_free->alloc_table, list)
    {
        vm_free_alloc(to_free, alloc->start, alloc->size);
        free_alloc(alloc);
    }
    H_FREE_TABLE(&to_free->alloc_table);
}

/** @brief Free all kernel memory associated with this ppd without locks
 *
 *  @param to_free The ppd to free
 *  @return void
 **/
void _free_ppd_kernel_mem(ppd_t* to_free)
{
    int i;
    page_directory_t* dir = to_free->dir;
    mutex_destroy(&to_free->lock);
    for (i = 0; i < TABLES_PER_DIR; i++) {
        // Check if it is a present user directory entry
        entry_t* dir_entry = &dir->tables[i];
        if (!is_present_user(dir_entry)) {
            continue;
        }
        void* addr = get_entry_address(*dir_entry);
        _sfree(addr, PAGE_SIZE);
    }
    _sfree(to_free->dir, PAGE_SIZE);
    _sfree(to_free, sizeof(ppd_t));
}

/** @brief Free all kernel memory associated with this ppd
 *
 *  @param to_free The ppd to free
 *  @return void
 **/
void free_ppd_kernel_mem(ppd_t* to_free)
{
    acquire_malloc();
    _free_ppd_kernel_mem(to_free);
    release_malloc();
}

/** @brief Switch dir and ppd
 *
 *  @param current The current ppd
 *  @param new The page directory to replace it with
 *  @return void
 **/
void switch_dir_ppd(ppd_t* current, page_directory_t* new)
{
    disable_interrupts();
    current->dir = new;
    switch_ppd(current);
    enable_interrupts();
}

/** @brief Free all memory associated with a ppd
 *
 *  @param to_free The ppd to free
 *  @param current The ppd currently in use (cannot be the same as to_free)
 *  @return void
 **/
void free_ppd(ppd_t* to_free, ppd_t* current)
{
    page_directory_t* tmp = current->dir;

    // switch page directory to free user memory
    switch_dir_ppd(current, to_free->dir);
    free_ppd_user_mem(to_free);
    switch_dir_ppd(current, tmp);

    free_ppd_kernel_mem(to_free);
}

/** @brief Switch from the current ppd to a supplied ppd
 *
 *  @param ppd The ppd to switch to
 *  @return void
 **/
void switch_ppd(ppd_t* ppd)
{
    set_cr3((uint32_t)ppd->dir);
}

/** @brief Initialize a ppd by copying all contents of another ppd
 *
 *  @param from The ppd to copy during initialization
 *  @return The new ppd or NULL on failure
 **/
ppd_t* init_ppd_from(ppd_t* from)
{
    assert(get_tcb()->process->directory == from);
    // create inital ppd
    ppd_t* ppd = init_ppd();
    if (ppd == NULL) {
        return NULL;
    }
    //copy over list of allocations
    if (copy_alloc_list(ppd, from) < 0) {
        free_ppd_kernel_mem(ppd);
        return NULL;
    }
    // swap to kernel ppd for copying
    page_directory_t* from_dir = from->dir;

    //temporarily use identity mapping
    switch_dir_ppd(from, virtual_memory.identity);
    int status = copy_page_dir(ppd->dir, from_dir);
    switch_dir_ppd(from, from_dir);

    if (status < 0) {
        free_ppd(ppd, from);
        return NULL;
    } else {
        return ppd;
    }
}
