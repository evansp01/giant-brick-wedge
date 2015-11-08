#ifndef PAGE_STRUCTS_H_
#define PAGE_STRUCTS_H_

#include <variable_queue.h>
#include <mutex.h>
#include <page.h>
#include <stdint.h>
#include <stddef.h>

typedef struct page_directory page_directory_t;

Q_NEW_HEAD(alloc_list_t, alloc);

typedef struct alloc {
    Q_NEW_LINK(alloc) list;
    uint32_t start;
    uint32_t size;
} alloc_t;

typedef struct ppd {
    page_directory_t* dir;
    int frames;
    alloc_list_t allocations;
    mutex_t lock;
} ppd_t;

void init_virtual_memory();

int init_ppd(ppd_t* ppd);
int init_ppd_from(ppd_t *ppd, ppd_t *from);
int free_ppd(ppd_t* to_free, ppd_t *current);
void switch_ppd(ppd_t* ppd);

int vm_alloc_readwrite(ppd_t *ppd, void *start, uint32_t size);
int vm_free(ppd_t *ppd, void *start);

int vm_user_strlen(ppd_t *ppd, char* start);
int vm_user_arrlen(ppd_t *ppd, char** start);

int vm_set_readonly(ppd_t *ppd, void* start, uint32_t size);
int vm_set_readwrite(ppd_t *ppd, void* start, uint32_t size);

int vm_user_can_read(ppd_t *ppd, void* start, uint32_t size);
int vm_user_can_write(ppd_t *ppd, void* start, uint32_t size);
int vm_user_can_alloc(ppd_t *ppd, void* start, uint32_t size);

int vm_write(ppd_t *ppd, void* buffer, void* start, uint32_t size);
int vm_read(ppd_t *ppd, void* buffer, void* start, uint32_t size);

int vm_resolve_pagefault(ppd_t *ppd, uint32_t cr2, int error_code);

#endif // PAGE_STRUCTS_H_
