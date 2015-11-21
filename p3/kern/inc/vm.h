/** @file vm.h
 *  @brief Interface for VM related functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_VM_H
#define KERN_INC_VM_H

#include <variable_queue.h>
#include <mutex.h>
#include <page.h>
#include <stdint.h>
#include <stddef.h>

typedef struct page_directory page_directory_t;

Q_NEW_HEAD(alloc_list_t, alloc);

/** @brief Struct for allocated frames */
typedef struct alloc {
    Q_NEW_LINK(alloc) list;
    uint32_t start;
    uint32_t size;
} alloc_t;

/** @brief Struct for page directories */
typedef struct ppd {
    page_directory_t* dir;
    int frames;
    alloc_list_t allocations;
    mutex_t lock;
} ppd_t;

void init_virtual_memory();
uint32_t page_align(uint32_t address);

ppd_t *init_ppd();
ppd_t *init_ppd_from(ppd_t *from);
void free_ppd(ppd_t* to_free, ppd_t* current);
void free_ppd_user_mem(ppd_t *to_free);
void free_ppd_kernel_mem(ppd_t* to_free);
void _free_ppd_kernel_mem(ppd_t* to_free);
void switch_ppd(ppd_t* ppd);

int vm_alloc_readwrite(ppd_t *ppd, void *start, uint32_t size);
int vm_back(ppd_t *ppd, uint32_t start, uint32_t size);
int vm_free(ppd_t *ppd, void *start);

int vm_user_strlen(ppd_t *ppd, char* start, int max_len);
int vm_user_arrlen(ppd_t* ppd, char** start, int max_len);

int vm_set_readonly(ppd_t *ppd, void* start, uint32_t size);
int vm_set_readwrite(ppd_t *ppd, void* start, uint32_t size);

int vm_user_can_read(ppd_t *ppd, void* start, uint32_t size);
int vm_user_can_write(ppd_t *ppd, void* start, uint32_t size);
int vm_user_can_alloc(ppd_t *ppd, void* start, uint32_t size);

int vm_write(ppd_t *ppd, void* buffer, void* start, uint32_t size);
int vm_read(ppd_t *ppd, void* buffer, void* start, uint32_t size);
int vm_read_locked(ppd_t* ppd, void* buffer, uint32_t start, uint32_t size);
int vm_write_locked(ppd_t* ppd, void* buffer, uint32_t start, uint32_t size);

int vm_resolve_pagefault(ppd_t *ppd, uint32_t cr2, int error_code);

#endif // KERN_INC_VM_H
