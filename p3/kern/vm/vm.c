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

COMPILE_TIME_ASSERT(sizeof(address_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(entry_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(page_directory_t) == PAGE_SIZE);
COMPILE_TIME_ASSERT(sizeof(page_table_t) == PAGE_SIZE);

#define KERNEL_TABLES DIVIDE_ROUND_UP(USER_MEM_START, PAGE_SIZE* PAGES_PER_TABLE)

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

struct {
    void* zero_page;
    page_table_t* kernel_pages[KERNEL_TABLES];
} virtual_memory = { 0 };

/** @brief Turns on virtual memory on the kernel
 *  @param dir Page directory base
 *  @return void
 **/
void turn_on_vm(page_directory_t* dir)
{
    set_cr3((uint32_t)dir);
    set_cr4(get_cr4() | CR4_PGE);
    set_cr0(get_cr0() | CR0_PG);
}

/** @brief Zero a frame
 *  @param frame The frame to zero
 *  @return void
 **/
void zero_frame(void* frame)
{
    ASSERT_PAGE_ALIGNED(frame);
    memset(frame, 0, PAGE_SIZE);
}

entry_t combine_permissions(entry_t* dir, entry_t* table)
{
    entry_t combined = { 0 };
    combined.user = dir->user & table->user;
    combined.write = dir->write & table->write;
    return combined;
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
    void* table_start = (void*)(table_idx * PAGE_SIZE * PAGES_PER_TABLE);
    //allocate a frame for the page table
    page_table_t* table = frame;
    //iterate over the physical pages which belong in this table
    for (page_idx = 0; page_idx < pages; page_idx++) {
        //add the page to the page table
        void* page = LEA(table_start, PAGE_SIZE, page_idx);
        table->pages[page_idx] = create_entry(page, init);
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
    int i;
    if (reserve_frames(1 + KERNEL_TABLES) < 0) {
        panic("Not enough frames to initialize virtual memory");
    }
    // set up the zero pages
    virtual_memory.zero_page = get_reserved_frame();
    // set up the page tables which define the kernel memory
    for (i = 0; i < KERNEL_TABLES; i++) {
        page_table_t* table = get_reserved_frame();
        table = physical_table(table, i, PAGES_PER_TABLE, e_kernel_global);
        virtual_memory.kernel_pages[i] = table;
    }
}

/** @brief Allocate and initialize a page directory
 *  @return the page directory
 **/
page_directory_t* create_page_directory()
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
page_table_t* create_page_table()
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
page_directory_t* create_kernel_directory()
{
    page_directory_t* dir = create_page_directory();
    if (dir == NULL) {
        panic("Could not allocate frame for kernel page directory");
    }

    int i;
    int machine_full_tables = machine_phys_frames() / PAGES_PER_TABLE;
    int machine_extra_pages = machine_phys_frames() % PAGES_PER_TABLE;

    for (i = KERNEL_TABLES; i < machine_full_tables; i++) {
        page_table_t* table = create_page_table();
        if (table == NULL) {
            panic("Could not allocate frame for kernel page table");
        }
        table = physical_table(table, i, PAGES_PER_TABLE, e_kernel_local);
        dir->tables[i] = create_entry(table, e_kernel_dir);
    }
    if (machine_extra_pages != 0) {
        page_table_t* table = create_page_table();
        if (table == NULL) {
            panic("Could not allocate frame for kernel page table");
        }
        table = physical_table(table, i, machine_extra_pages, e_kernel_local);
        dir->tables[i] = create_entry(table, e_kernel_dir);
    }
    return dir;
}

/** @brief Allocates all pages in page table from start address to start+size
 *  @param cr2 The address of the page table
 *  @param start The virtual address to begin allocation at
 *  @param size The amount of virtual memory to allocate pages for
 *  @param model The permissions to use for created page table entries
 *  @return zero on success less than zero on failure
 **/
int allocate_pages(void* cr2, void* start, size_t size, entry_t model)
{
    page_directory_t* dir = cr2;
    char* end = ((char*)start) + size;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    address_t location = { 0 };
    int i;
    // allocate all relevant page tables
    for (i = vm_start.page_dir_index; i <= vm_end.page_dir_index; i++) {
        location.page_dir_index = i;
        entry_t* dir_entry = &dir->tables[i];
        if (!dir_entry->present) {
            void* frame = create_page_table();
            if (frame == NULL) {
                lprintf("Ran out of kernel memory for page tables");
                return -1;
            }
            *dir_entry = create_entry(frame, e_user_dir);
        }
        int j;
        int start_index = 0;
        int end_index = PAGES_PER_TABLE - 1;
        page_table_t* table = get_entry_address(*dir_entry);
        if (i == vm_start.page_dir_index) {
            start_index = vm_start.page_table_index;
        }
        if (i == vm_end.page_dir_index) {
            end_index = vm_end.page_table_index;
        }
        for (j = start_index; j <= end_index; j++) {
            location.page_table_index = j;
            entry_t* table_entry = &table->pages[j];
            if (table_entry->present) {
                lprintf("WARN: page already allocated at %x",
                        AS_TYPE(location, int));
                continue;
            }
            void* frame = allocate_frame();
            if (frame == NULL) {
                lprintf("Ran out of frames to allocate");
                return -2;
            }
            *table_entry = create_entry(frame, model);
            zero_frame(AS_TYPE(location, void*));
        }
    }
    return 0;
}

/** @brief Gets or creates the physical mapping of a virtual address
 *  @param cr2 The address of the page table to use
 *  @param virtual The virtual address to write to
 *  @param physical A place to store the write pointer
 *  @param model A model entry used to create missing page table entries
 *  @param permissions If not null will be set to permissions of this page
 *  @return Bytes to the end of the page, or less than zero on error
 **/
int vm_to_physical_create(void* cr2, void* virtual, entry_t model,
                          void** physical, entry_t* permissions)
{
    ASSERT_PAGE_ALIGNED(cr2);
    page_directory_t* dir = cr2;
    entry_t* dir_entry = get_dir_entry(virtual, dir);
    if (!dir_entry->present) {
        void* frame = create_page_table();
        if (frame == NULL) {
            return -1;
        }
        *dir_entry = create_entry(frame, e_user_dir);
    }
    page_table_t* table = get_entry_address(*dir_entry);
    entry_t* table_entry = get_table_entry(virtual, table);
    if (!table_entry->present) {
        void* frame = allocate_frame();
        if (frame == NULL) {
            return -1;
        }
        *table_entry = create_entry(frame, model);
    }
    void* address = get_address(virtual, get_entry_address(*table_entry));
    *physical = address;
    if (permissions != NULL) {
        *permissions = combine_permissions(dir_entry, table_entry);
    }
    return page_bytes_left(virtual);
}

/** @brief Gets the physical mapping of a virtual address
 *  @param cr2 The page table register
 *  @param virtual The virtual address
 *  @param physical The physical address
 *  @param permissions If not null will be set to permissions of this page
 *  @return less than zero on error, otherwise bytes to end of page
 **/
int vm_to_physical(void* cr2, void* virtual,
                   void** physical, entry_t* permissions)
{
    ASSERT_PAGE_ALIGNED(cr2);
    page_directory_t* dir = cr2;
    entry_t* dir_entry = get_dir_entry(virtual, dir);
    if (!dir_entry->present) {
        return -1;
    }
    page_table_t* table = get_entry_address(*dir_entry);
    entry_t* table_entry = get_table_entry(virtual, table);
    if (!table_entry->present) {
        return -2;
    }
    void* address = get_address(virtual, get_entry_address(*table_entry));
    *physical = address;
    if (permissions != NULL) {
        *permissions = combine_permissions(dir_entry, table_entry);
    }
    return page_bytes_left(virtual);
}

#define MIN(x, y) ((x) < (y) ? (x) : (y))

/** @brief Write from a buffer to a virtual memory address
 *  @param cr2 The page directory address of virtual memory
 *  @param address The address in virtual memory to write to
 *  @param buffer The buffer to write from
 *  @param size The amount of memory to write
 **/
int vm_write(void* cr2, void* address, void* buffer, int size)
{
    int to_write = size;
    char* offset = buffer;
    char* virtual = address;
    while (to_write > 0) {
        void* physical;
        int bytes = vm_to_physical_create(cr2, virtual, e_write_page, &physical, NULL);
        if (bytes < 0) {
            return size - to_write;
        }
        int write_size = MIN(to_write, bytes);
        memcpy(physical, offset, write_size);
        to_write -= write_size;
        offset += write_size;
        virtual += write_size;
    }
    return size;
}

/** @brief Write from a buffer to a virtual memory address
 *  @param cr2 The page directory address of virtual memory
 *  @param address The address in virtual memory to read from
 *  @param buffer The buffer to read to
 *  @param size The amount of memory to read
 **/
int vm_read(void* cr2, void* address, void* buffer, int size)
{
    int to_read = size;
    char* offset = buffer;
    char* virtual = address;
    while (to_read > 0) {
        void* physical;
        int bytes = vm_to_physical(cr2, virtual, &physical, NULL);
        if (bytes < 0) {
            return size - to_read;
        }
        int read_size = MIN(to_read, bytes);
        memcpy(offset, physical, read_size);
        to_read -= read_size;
        offset += read_size;
        virtual += read_size;
    }
    return size;
}
