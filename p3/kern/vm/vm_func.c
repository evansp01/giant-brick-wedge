#include <vm.h>
#include <utilities.h>
#include <contracts.h>
#include <string.h>
#include "vm_internal.h"

/** @brief Is this page present and user accessable
 *
 *  @param entry The page table entry for the page
 *  @return an integer boolean
 **/
int is_present_user(entry_t* entry)
{
    return entry->present && entry->user;
}

/** @brief Is this page user accessable
 *
 *  @param table The page table entry for this page
 *  @param dir The page directory entry for the page table
 *  @return a boolean integer
 **/
int is_user(entry_t* table, entry_t* dir)
{
    return table->present && table->user && dir->user;
}

/** @brief Is this page write accessable
 *
 *  @param table The page table entry for this page
 *  @return a boolean integer
 **/
int is_write(entry_t* table)
{
    return table->write || (is_zfod(table) && table->zfod);
}

/** @brief Is this page the zfod page
 *
 *  @param table The page table entry for this page
 *  @return a boolean integer
 **/
int is_zfod(entry_t* table)
{
    return (get_entry_address(*table) == get_zero_page());
}

/** @brief mapper for map pages */
typedef int (*vm_operator)(entry_t*, entry_t*, address_t);

/** @brief Map a mapper function across a range of pages
 *
 *  @ppd The page directory to map across
 *  @start The starting address to map from
 *  @size The size to map
 *  @op The vm mapper to run
 *  @return Zero on success an integer less than zero on failure
 **/
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
/** @brief Determine if a set of pages is allocatable
 *
 *  @ppd The page directory to map across
 *  @start The starting address to map from
 *  @size The size to map
 *  @return an integer boolean
 **/
int vm_user_can_alloc(ppd_t* ppd, void* start, uint32_t size)
{
    int i, j;
    page_directory_t* dir = ppd->dir;
    char* end = ((char*)start) + size - 1;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    if (AS_TYPE(vm_start, uint32_t) > AS_TYPE(vm_end, uint32_t)) {
        lprintf("Something");
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

/** @brief Get all the information about an address present in the page table
 *
 *  @param ppd The page directory to get information from
 *  @param addr The address to get information about
 *  @param table A pointer which will point to the address's page table entry
 *  @param dir A pointer which will point to the address's page directory entry
 *  @return Zero on success an integer less than zero on failure
 **/
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

/** @brief Safely calculate the length of a userspace string
 *
 *  @param ppd The page directory of the userspace program
 *  @param start The starting address of the string
 *  @return The length of the string on success, less than zero on failure
 **/
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

/** @brief Safely calculate the length of a userspace string array
 *
 *  @param ppd The page directory of the userspace program
 *  @param start The starting address of the string array
 *  @return The length of the string on success, less than zero on failure
 **/
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

/** @brief A vm_operator to determine if pages are user writeable
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_user_write_h(entry_t* table, entry_t* dir, address_t addr)
{

    if (is_user(table, dir) && is_write(table)) {
        return 0;
    }
    return -3;
}

/** @brief A vm_operator to determine if pages are user readable
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_user_read_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (is_user(table, dir)) {
        return 0;
    }
    return -3;
}

/** @brief A vm_operator to make user read pages writeable
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_set_readwrite_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (!is_write(table)) {
        if (is_zfod(table)) {
            // set the zfod write bit
            table->zfod = 1;
            ASSERT(table->write == 0);
        } else {
            // set the write bit
            table->write = 1;
            ASSERT(table->zfod == 0);
        }
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

/** @brief A vm_operator to make user readwrite pages read only
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_set_readonly_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (is_write(table)) {
        if (is_zfod(table)) {
            table->zfod = 0;
            ASSERT(table->write == 0);
        } else {
            table->write = 0;
            ASSERT(table->zfod == 0);
        }
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

/** @brief A vm_operator to allocate pages as readwrite using zfod
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_alloc_readwrite_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (table->present) {
        lprintf("Error already allocated");
        return -1;
    }
    *table = create_entry(get_zero_page(), e_zfod_page);
    return 0;
}

/** @brief A vm_operator which backs zfod pages with real frames
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_back_h(entry_t* table, entry_t* dir, address_t addr)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (is_zfod(table)) {
        if (is_write(table)) {
            return alloc_frame(AS_TYPE(addr, void*), table, e_write_page);
        } else {
            return alloc_frame(AS_TYPE(addr, void*), table, e_read_page);
        }
    }
    return 0;
}

/** @brief A vm_operator to free a user page
 *
 *  @param table The table entry for the current page
 *  @param dir The directory entry for the current page
 *  @param addr The virtual address of the current page
 *  @return Zero to continue, less than zero to stop iteration and return false
 **/
int vm_free_alloc_h(entry_t* table, entry_t* dir, address_t addr)
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
    // make sure we can write to the page
    table->write = 1;
    invalidate_page(virtual);
    free_frame(virtual, get_entry_address(*table));
    *table = e_unmapped;
    invalidate_page(virtual);
    return 0;
}

/** @brief Can a user write to a given set of addresses
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return an integer boolean
 **/
int vm_user_can_write(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_user_write_h) == 0;
}

/** @brief Can a user read from a given set of addresses
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return an integer boolean
 **/
int vm_user_can_read(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_user_read_h) == 0;
}

/** @brief Set a group of user pages to be user readable and writable
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return an integer boolean
 **/
int vm_set_readwrite(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_set_readwrite_h) == 0;
}

/** @brief Set a group of user pages to be user read only
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return an integer boolean
 **/
int vm_set_readonly(ppd_t* ppd, void* start, uint32_t size)
{
    return vm_map_pages(ppd, start, size, vm_set_readonly_h) == 0;
}

/** @brief Safely read from user memory to a kernel buffer using the ppd lock
 *
 *  @param ppd The user page directory
 *  @param buffer The buffer to read to
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_read_locked(ppd_t* ppd, void* buffer, uint32_t start, uint32_t size)
{
    mutex_lock(&ppd->lock);
    int status = vm_read(ppd, buffer, (void*)start, size);
    mutex_unlock(&ppd->lock);
    return status;
}

/** @brief Safely read from user memory to a kernel buffer
 *
 *  @param ppd The user page directory
 *  @param buffer The buffer to read to
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_read(ppd_t* ppd, void* buffer, void* start, uint32_t size)
{
    if (vm_user_can_read(ppd, start, size)) {
        memcpy(buffer, start, size);
        return 0;
    }
    return -1;
}

/** @brief Safely write from a kernel buffer to user memory using the ppd lock
 *
 *  @param ppd The user page directory
 *  @param buffer The buffer to write from
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_write_locked(ppd_t* ppd, void* buffer, uint32_t start, uint32_t size)
{
    mutex_lock(&ppd->lock);
    int status = vm_write(ppd, buffer, (void*)start, size);
    mutex_unlock(&ppd->lock);
    return status;
}

/** @brief Safely write from a kernel buffer to user memory
 *
 *  @param ppd The user page directory
 *  @param buffer The buffer to write from
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_write(ppd_t* ppd, void* buffer, void* start, uint32_t size)
{
    if (vm_user_can_write(ppd, start, size)) {
        if (vm_back(ppd, (uint32_t)start, size) < 0) {
            return -1;
        }
        // at this point we should not page fault
        memcpy(start, buffer, size);
        return 0;
    }
    return -1;
}

/** @brief Allocate a section of userspace memory using zfod
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_alloc_readwrite(ppd_t* ppd, void* start, uint32_t size)
{
    //allocations of size zero are successful and also noops
    if (size == 0) {
        return 0;
    }
    //you can't allocate more memory than the system has
    if (reserve_frames(start, size) < 0) {
        return -1;
    }
    if (allocate_tables(ppd, start, size) < 0) {
        return -1;
    }
    if (add_alloc(ppd, start, size) < 0) {
        return -1;
    }
    // at this point all memory is allocated
    ASSERT(vm_map_pages(ppd, start, size, vm_alloc_readwrite_h) >= 0);
    return 0;
}

/** @brief Back an allocated section of userspace memory with physical pages
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_back(ppd_t* ppd, uint32_t start, uint32_t size)
{
    return vm_map_pages(ppd, (void*)start, size, vm_back_h);
}

/** @brief Free a previously allocated a section of userspace memory
 *
 *  @param ppd The user page directory
 *  @param start The start address
 *  @param size The size of the section to check
 *  @return Zero on success, an integer less than zero on failure
 **/
int vm_free_alloc(ppd_t* ppd, uint32_t start, uint32_t size)
{
    release_frames((void*)start, size);
    return vm_map_pages(ppd, (void*)start, size, vm_free_alloc_h);
}
