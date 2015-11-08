#include <vm.h>
#include <mutex.h>
#include <common.h>
#include <cr.h>
#include "vm_internal.h"
#include <stdlib.h>
#include <malloc.h>

int init_ppd(ppd_t* ppd)
{
    ppd->frames = 0;
    Q_INIT_HEAD(&ppd->allocations);
    mutex_init(&ppd->lock);
    if ((ppd->dir = alloc_page_directory()) == NULL) {
        return -1;
    }
    return 0;
}

int add_alloc(ppd_t* ppd, void* start, uint32_t size)
{
    alloc_t* new_alloc = malloc(sizeof(alloc_t));
    if (new_alloc == NULL) {
        return -1;
    }
    Q_INIT_ELEM(new_alloc, list);
    new_alloc->start = (uint32_t)start;
    new_alloc->size = size;
    Q_INSERT_TAIL(&ppd->allocations, new_alloc, list);
    return 0;
}

int vm_free(ppd_t* ppd, void* start)
{
    alloc_t* alloc;
    int found = 0;
    Q_FOREACH(alloc, &ppd->allocations, list)
    {
        if (alloc->start == (uint32_t)start) {
            found = 1;
            break;
        }
    }
    if (!found) {
        return -1;
    }
    Q_REMOVE(&ppd->allocations, alloc, list);
    vm_free_alloc(ppd, alloc->start, alloc->size);
    free(alloc);
    return 0;
}

int free_ppd(ppd_t* to_free, ppd_t *current)
{
    alloc_t* alloc;
    alloc_t* tmp;
    switch_ppd(to_free);
    Q_FOREACH_SAFE(alloc, tmp, &to_free->allocations, list)
    {
        Q_REMOVE(&to_free->allocations, alloc, list);
        vm_free_alloc(to_free, alloc->start, alloc->size);
        free(alloc);
    }
    free_page_directory(to_free->dir);
    switch_ppd(current);
    return 0;
}

void switch_ppd(ppd_t* ppd)
{
    set_cr3((uint32_t)ppd->dir);
}

void copy_alloc(alloc_t* to, alloc_t* from)
{
    to->start = from->start;
    to->size = from->size;
}

int init_ppd_from(ppd_t* ppd, ppd_t* from)
{
    // create inital ppd
    if (init_ppd(ppd) < 0) {
        return -1;
    }
    ppd->frames = from->frames;
    //copy over list of allocations
    alloc_t* alloc;
    Q_FOREACH(alloc, &from->allocations, list)
    {
        alloc_t* copy = malloc(sizeof(alloc_t));
        Q_INIT_ELEM(copy, list);
        if (copy == NULL) {
            return -1;
        }
        copy_alloc(copy, alloc);
        Q_INSERT_TAIL(&ppd->allocations, copy, list);
    }
    // swap to kernel ppd for copying
    page_directory_t* from_dir = from->dir;
    //temporarily use identity mapping
    from->dir = virtual_memory.identity;
    switch_ppd(from);
    int status = copy_page_dir(ppd->dir, from_dir);
    from->dir = from_dir;
    switch_ppd(from);
    return status;
}
