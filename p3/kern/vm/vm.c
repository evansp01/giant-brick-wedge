#include <stdint.h>
#include <stdlib.h>
#include <page.h>
#include <common_kern.h>
#include <string.h>
#include <utilities.h>
#include <stdio.h>
#include <vm.h>
#include <malloc.h>
#include <cr.h>
#include "vm_internal.h"

COMPILE_TIME_ASSERT(sizeof(address_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(entry_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(page_directory_t) == PAGE_SIZE);
COMPILE_TIME_ASSERT(sizeof(page_table_t) == PAGE_SIZE);

struct virtual_memory virtual_memory = { 0 };

const entry_t e_kernel_dir = {
    .present = 1,
    .write = 1
};

const entry_t e_user_dir = {
    .present = 1,
    .write = 1,
    .user = 1
};

const entry_t e_kernel_global = {
    .present = 1,
    .write = 1,
    .global = 1,
};

const entry_t e_kernel_local = {
    .present = 1,
    .write = 1,
};

const entry_t e_read_page = {
    .present = 1,
    .user = 1
};

const entry_t e_write_page = {
    .present = 1,
    .write = 1,
    .user = 1
};

const entry_t e_zfod_page = {
    .present = 1,
    .zfod = 1,
    .user = 1
};

const entry_t e_unmapped = { 0 };

void print_entry(entry_t* entry)
{
    lprintf("present:%d, write:%d, user:%d, write_through:%d",
            entry->present, entry->write, entry->user, entry->write_through);
    lprintf("cache_disable:%d, accessed:%d, dirty:%d, page_size:%d, global:%d",
            entry->cache_disable, entry->accessed, entry->dirty,
            entry->page_size, entry->global);
    lprintf("unused:%d, address:%d",
            entry->unused, entry->address);
}

entry_t create_entry(void* address, entry_t model)
{
    entry_t entry = model;
    entry.address = ((uint32_t)address) >> ENTRY_ADDRESS_SHIFT;
    return entry;
}

/** @brief Gets the page aligned address from an page table/dir entry
 *  @param entry The entry to get the address from
 *  @return the address
 **/
inline void* get_entry_address(entry_t entry)
{
    uint32_t addr = entry.address;
    return (void*)(addr << ENTRY_ADDRESS_SHIFT);
}

/** @brief Gets the appropriate entry from a page directory give an address
 *  @param address The address to use as a key
 *  @param directory The page directory
 *  @return The appropriate page directory entry
 **/
entry_t* get_dir_entry(void* address, page_directory_t* directory)
{
    ASSERT_PAGE_ALIGNED(directory);
    uint32_t offset = AS_TYPE(address, address_t).page_dir_index;
    return &directory->tables[offset];
}

/** @brief Gets the appropriate entry from a page table give an address
 *  @param address The address to use as a key
 *  @param table The page table
 *  @return The appropriate page table entry
 **/
entry_t* get_table_entry(void* address, page_table_t* table)
{
    ASSERT_PAGE_ALIGNED(table);
    uint32_t offset = AS_TYPE(address, address_t).page_table_index;
    return &table->pages[offset];
}

/** @brief Get the physical address given a virtual address and its page
 *  @param address The virtual address
 *  @param page The appropriate page from the page table
 *  @return The physical address
 **/
void* get_address(void* address, void* page)
{
    ASSERT_PAGE_ALIGNED(page);
    uint32_t offset = AS_TYPE(address, address_t).page_address;
    return (void*)(((uint32_t)page) | offset);
}

int page_bytes_left(void* address)
{
    uint32_t offset = AS_TYPE(address, address_t).page_address;
    return PAGE_SIZE - offset;
}

/** @brief Create a page table with the identiy mapping to the physical pages
 *
 *  The table_idx is the index into the page directory. This index is also
 *  used to determine which physical page we should start at.
 *
 *  @param frame The frame to use for the table, must be zeroed
 *  @param table_idx The index into the page directory
 *  @param pages The number of pages to create entries for
 *  @param init Initial values for flags in the page table and dir entries
 *  @return The created table
 **/
page_table_t*
physical_table(void* frame, int table_idx, int pages, entry_t init)
{
    ASSERT_PAGE_ALIGNED(frame);
    int page_idx;
    //get the address of the first page this page table will contain
    uint32_t table_start = table_idx * PAGE_SIZE * PAGES_PER_TABLE;
    //allocate a frame for the page table
    page_table_t* table = frame;
    //iterate over the physical pages which belong in this table
    for (page_idx = 0; page_idx < pages; page_idx++) {
        //add the page to the page table
        uint32_t page = table_start + PAGE_SIZE * page_idx;
        table->pages[page_idx] = create_entry((void*)page, init);
    }
    return table;
}

/** @brief Performs initialization that virtual memory functions require
 *
 *  This function sets up the special pages virtual memory reserves like
 *  the zero page and the kernel mapping pages
 *
 *  @return void
 **/
void init_virtual_memory()
{
    init_frame_alloc();
    int i;
    // set up the page tables which define the kernel memory
    for (i = 0; i < KERNEL_TABLES; i++) {
        page_table_t* table = (page_table_t*)smemalign(PAGE_SIZE, PAGE_SIZE);
        table = physical_table(table, i, PAGES_PER_TABLE, e_kernel_global);
        virtual_memory.kernel_pages[i] = table;
    }
    virtual_memory.identity = alloc_kernel_directory();
    set_cr3((uint32_t)virtual_memory.identity);
    set_cr4(get_cr4() | CR4_PGE);
    set_cr0(get_cr0() | CR0_PG | CR0_WP);
}

/** @brief Allocate and initialize a page directory
 *  @return the page directory
 **/
page_directory_t* alloc_page_directory()
{
    int i;
    page_directory_t* dir = (page_directory_t*)smemalign(PAGE_SIZE, PAGE_SIZE);
    if (dir == NULL) {
        return NULL;
    }
    zero_frame(dir);
    //map in the global kernel pages
    for (i = 0; i < KERNEL_TABLES; i++) {
        void* kernel_page = virtual_memory.kernel_pages[i];
        dir->tables[i] = create_entry(kernel_page, e_kernel_dir);
    }
    return dir;
}

/** @brief Allocate and initialize a page table
 *  @return the page table
 **/
page_table_t* alloc_page_table()
{
    page_table_t* table = (page_table_t*)smemalign(PAGE_SIZE, PAGE_SIZE);
    if (table == NULL) {
        return NULL;
    }
    zero_frame(table);
    return table;
}

/** @brief Create a page directory for the kernel with the identity mapping
 *  @return The page directory
 **/
page_directory_t* alloc_kernel_directory()
{
    page_directory_t* dir = alloc_page_directory();
    if (dir == NULL) {
        panic("Could not allocate frame for kernel page directory");
    }

    int i;
    int machine_full_tables = machine_phys_frames() / PAGES_PER_TABLE;
    int machine_extra_pages = machine_phys_frames() % PAGES_PER_TABLE;

    for (i = KERNEL_TABLES; i < machine_full_tables; i++) {
        page_table_t* table = alloc_page_table();
        if (table == NULL) {
            panic("Could not allocate frame for kernel page table");
        }
        table = physical_table(table, i, PAGES_PER_TABLE, e_kernel_local);
        dir->tables[i] = create_entry(table, e_kernel_dir);
    }
    if (machine_extra_pages != 0) {
        page_table_t* table = alloc_page_table();
        if (table == NULL) {
            panic("Could not allocate frame for kernel page table");
        }
        table = physical_table(table, i, machine_extra_pages, e_kernel_local);
        dir->tables[i] = create_entry(table, e_kernel_dir);
    }
    return dir;
}

int is_present_user(entry_t* entry)
{
    return entry->present && entry->user;
}

int copy_frame(entry_t* child_entry, entry_t* parent_entry)
{
    if (is_zfod(parent_entry)) {
        *child_entry = create_entry(get_zero_page(), *parent_entry);
        return 0;
    }
    // Create copy of page
    if (kernel_alloc_frame(child_entry, *parent_entry) < 0) {
        lprintf("Ran out of frames to allocate");
        return -1;
    }
    // Copy frame data
    memcpy(get_entry_address(*child_entry),
           get_entry_address(*parent_entry), PAGE_SIZE);
    return 0;
}

/** @brief Copies the page frames into a new process
 *
 *  @param table_parent PCB of parent process
 *  @param table_child PCB of child process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_page_table(page_table_t* table_child, page_table_t* table_parent)
{
    int i_page;
    for (i_page = 0; i_page < PAGES_PER_TABLE; i_page++) {

        // Check if it is a present user page table entry
        entry_t* parent_entry = &table_parent->pages[i_page];
        if (!is_present_user(parent_entry)) {
            continue;
        }
        entry_t* child_entry = &table_child->pages[i_page];
        if (copy_frame(child_entry, parent_entry) < 0) {
            return -1;
        }
    }
    return 0;
}

/** @brief Copies the page tables into a new process
 *
 *  @param dir_child PCB of child process
 *  @param dir_parent PCB of parent process
 *  @return Zero on success, an integer less than zero on failure
 **/
int copy_page_dir(page_directory_t* dir_child, page_directory_t* dir_parent)
{
    // Copy memory regions
    int i_dir;
    for (i_dir = 0; i_dir < TABLES_PER_DIR; i_dir++) {
        // Check if it is a present user directory entry
        entry_t* dir_entry_parent = &dir_parent->tables[i_dir];
        if (!is_present_user(dir_entry_parent)) {
            continue;
        }

        // Create copy of page table
        entry_t* dir_entry_child = &dir_child->tables[i_dir];
        void* table = alloc_page_table();
        if (table == NULL) {
            lprintf("Ran out of kernel memory for page tables");
            return -1;
        }
        *dir_entry_child = create_entry(table, *dir_entry_parent);
        // Get page table
        page_table_t* table_child = get_entry_address(*dir_entry_child);
        page_table_t* table_parent = get_entry_address(*dir_entry_parent);

        // Copy page frames from parent to child
        if (copy_page_table(table_child, table_parent) < 0) {
            return -2;
        }
    }
    return 0;
}

void free_page_directory(page_directory_t* dir)
{
    int i;
    for (i = 0; i < TABLES_PER_DIR; i++) {
        // Check if it is a present user directory entry
        entry_t* dir_entry = &dir->tables[i];
        if (!is_present_user(dir_entry)) {
            continue;
        }
        void *addr = get_entry_address(*dir_entry);
        sfree(addr, PAGE_SIZE);
    }
    sfree(dir, PAGE_SIZE);
}

