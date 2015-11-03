#include <vm.h>
#include <mutex.h>
#include <common.h>
#include <cr.h>
#include "vm_internal.h"
#include <stdlib.h>

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

int free_ppd(ppd_t* ppd)
{
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
