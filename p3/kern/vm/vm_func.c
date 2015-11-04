#include <vm.h>
#include <utilities.h>
#include <string.h>
#include "vm_internal.h"

typedef int (*vm_operator)(entry_t*, entry_t*, address_t);

int vm_map_pages(ppd_t* ppd, void* start, uint32_t size, vm_operator op)
{
    int i, j, value = 0;
    page_directory_t* dir = ppd->dir;
    char* end = ((char*)start) + size - 1;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    if (AS_TYPE(vm_start, uint32_t) > AS_TYPE(vm_end, uint32_t)) {
        return -1;
    }
    for (i = vm_start.page_dir_index; i <= vm_end.page_dir_index; i++) {
        entry_t* dir_entry = &dir->tables[i];
        if (!dir_entry->present) {
            lprintf("Page dir entry not present");
            return -1;
        }
        int start_index = 0;
        int end_index = PAGES_PER_TABLE - 1;
        if (i == vm_start.page_dir_index) {
            start_index = vm_start.page_table_index;
        }
        if (i == vm_end.page_dir_index) {
            end_index = vm_end.page_table_index;
        }
        page_table_t* table = get_entry_address(*dir_entry);
        address_t location = { .page_dir_index = i };
        for (j = start_index; j <= end_index; j++) {
            location.page_table_index = j;
            entry_t* table_entry = &table->pages[j];
            if ((value = op(table_entry, dir_entry, location)) < 0) {
                return value;
            }
        }
    }
    return value;
}

int vm_user_can_alloc(ppd_t* ppd, void* start, uint32_t size)
{
    int i, j;
    page_directory_t* dir = ppd->dir;
    char* end = ((char*)start) + size;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    if (AS_TYPE(vm_start, uint32_t) > AS_TYPE(vm_end, uint32_t)) {
        return 0;
    }
    for (i = vm_start.page_dir_index; i <= vm_end.page_dir_index; i++) {
        entry_t* dir_entry = &dir->tables[i];
        if (!dir_entry->present) {
            continue;
        }
        if (!dir_entry->user) {
            //kernel only
            return 0;
        }
        int start_index = 0;
        int end_index = PAGES_PER_TABLE - 1;
        if (i == vm_start.page_dir_index) {
            start_index = vm_start.page_table_index;
        }
        if (i == vm_end.page_dir_index) {
            end_index = vm_end.page_table_index;
        }
        page_table_t* table = get_entry_address(*dir_entry);
        for (j = start_index; j <= end_index; j++) {
            entry_t* table_entry = &table->pages[j];
            if (!table_entry->present) {
                continue;
            }
            //something is present
            return 0;
        }
    }
    return 1;
}

int vm_get_address(ppd_t* ppd, void* addr, entry_t** table, entry_t** dir)
{
    page_directory_t* page_dir = ppd->dir;
    entry_t* dir_entry = get_dir_entry(addr, page_dir);
    if (!dir_entry->present) {
        return -1;
    }
    page_table_t* page_table = get_entry_address(*dir_entry);
    entry_t* table_entry = get_table_entry(addr, page_table);
    if (!table_entry->present) {
        return -1;
    }
    *table = table_entry;
    *dir = dir_entry;
    return page_bytes_left(addr);
}

int is_user(entry_t* table, entry_t* dir)
{
    return table->present && table->user && dir->user;
}

int is_write(entry_t* table)
{
    return table->write || table->zfod;
}

int is_zfod(entry_t* table)
{
    return (get_entry_address(*table) == get_zero_page());
}

int vm_user_strlen(ppd_t* ppd, char* start)
{
    entry_t* table, *dir;
    int space = 0;
    int i, length;
    for (;;) {
        if ((length = vm_get_address(ppd, start + space, &table, &dir)) < 0) {
            return -1;
        }
        if (!is_user(table, dir)) {
            return -1;
        }
        for (i = 0; i < length; i++) {
            if (start[space + i] == '\0') {
                return space + i;
            }
        }
        space += length;
    }
}

int vm_user_arrlen(ppd_t* ppd, char** start)
{
    entry_t* table, *dir;
    int space = 0;
    int i, length;
    for (;;) {
        if ((length = vm_get_address(ppd, start + space, &table, &dir)) < 0) {
            return -1;
        }
        if (!is_user(table, dir)) {
            return -1;
        }
        for (i = 0; i < length; i++) {
            if (start[space + i] == '\0') {
                return space + i;
            }
        }
        space += length;
    }
}

int vm_user_write_h(entry_t* table, entry_t* dir, address_t addr)
{

    if (is_user(table, dir) && is_write(table)) {
        return 0;
    }
    return -3;
}

int vm_user_read_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (is_user(table, dir)) {
        return 0;
    }
    return -3;
}

int vm_set_readwrite_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (!is_write(table)) {
        if (is_zfod(table)) {
            // set the zfod write bit
            table->zfod = 1;
        } else {
            // set the write bit
            table->write = 1;
        }
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

int vm_set_readonly_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    // no worries about zfod stuff
    if(is_write(table)){
        if(is_zfod(table)){
            table->zfod = 0;
        } else {
            table->write = 0;
        }
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

int vm_user_can_write(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_user_write_h) == 0;
}

int vm_user_can_read(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_user_read_h) == 0;
}

int vm_set_readwrite(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_set_readwrite_h) == 0;
}

int vm_set_readonly(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_set_readonly_h) == 0;
}

int vm_read(ppd_t* ppd, void* buffer, void* start, uint32_t size)
{
    if (vm_user_can_read(ppd, start, size)) {
        memcpy(buffer, start, size);
        return 0;
    }
    return -1;
}

int vm_write(ppd_t* ppd, void* buffer, void* start, uint32_t size)
{
    if (vm_user_can_write(ppd, start, size)) {
        memcpy(start, buffer, size);
        return 0;
    }
    return -1;
}

/** @brief Allocates all page tables from start address to start+size
 *  @param cr3 The address of the page table
 *  @param start The virtual address to begin allocation at
 *  @param size The amount of virtual memory to allocate pages for
 *  @return zero on success less than zero on failure
 **/
int allocate_tables(ppd_t* ppd, void* start, uint32_t size)
{
    int i;
    page_directory_t* dir = ppd->dir;
    char* end = ((char*)start) + size - 1;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    if (AS_TYPE(vm_start, uint32_t) > AS_TYPE(vm_end, uint32_t)) {
        return -1;
    }
    // allocate all relevant page tables
    for (i = vm_start.page_dir_index; i <= vm_end.page_dir_index; i++) {
        entry_t* dir_entry = &dir->tables[i];
        if (dir_entry->present) {
            continue;
        }
        void* frame = alloc_page_table();
        if (frame == NULL) {
            lprintf("Ran out of kernel memory for page tables");
            return -1;
        }
        *dir_entry = create_entry(frame, e_user_dir);
    }
    // at this point the allocation has committed and must be performed
    return 0;
}

int vm_alloc_readwrite_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (table->present) {
        lprintf("Error already allocated");
        return -1;
    }
    *table = create_entry(get_zero_page(), e_zfod_page);
    return 0;
}

int vm_alloc_readwrite(ppd_t* ppd, void* start, uint32_t size)
{
    //allocations of size zero are successful
    if (size == 0) {
        return 0;
    }
    //you can't allocate more memory than the system has
    if (size > user_mem_size()) {
        return -1;
    }
    if (allocate_tables(ppd, start, size) < 0) {
        return -1;
    }
    if (add_alloc(ppd, start, size) < 0) {
        return -1;
    }
    // at this point all memory is allocated
    if (vm_map_pages(ppd, start, size, vm_alloc_readwrite_h) < 0) {
        lprintf("This really shouldn't happen");
        return -2;
    }
    return 0;
}

int vm_free_h(entry_t* table, entry_t* dir, address_t addr)
{
    // everything we are freeing should be user mapped
    if (!is_user(table, dir)) {
        return -3;
    }
    void* virtual = AS_TYPE(addr, void*);
    // for zfod pages we can just delete the page
    if (is_zfod(table)) {
        *table = e_unmapped;
        invalidate_page(virtual);
        return 0;
    }
    // mark as kernel only to eliminate race conditions
    table->user = 0;
    invalidate_page(virtual);
    free_frame(virtual, get_entry_address(*table));
    *table = e_unmapped;
    invalidate_page(virtual);
    return 0;
}

int vm_free(ppd_t* ppd, void* start)
{
    uint32_t size;
    if (remove_alloc(ppd, start, &size) < 0) {
        return -1;
    }
    return vm_map_pages(ppd, start, size, vm_free_h);
}
