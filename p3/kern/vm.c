#include <stdint.h>
#include <page.h>
#include <common_kern.h>
#include <string.h>
#include <utilities.h>
#include <page_structs.h>

COMPILE_TIME_ASSERT(sizeof(address_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(entry_t) == sizeof(uint32_t));
COMPILE_TIME_ASSERT(sizeof(page_directory_t) == PAGE_SIZE);
COMPILE_TIME_ASSERT(sizeof(page_table_t) == PAGE_SIZE);


/** @brief Gets the page aligned address from an page table/dir entry
 *  @param entry The entry to get the address from
 *  @return the address
 **/
inline void* get_entry_address(entry_t entry)
{
    uint32_t addr = entry.address;
    return (void*)(addr << ENTRY_ADDRESS_SHIFT);
}

/** @brief Set the page aligned address in a page table/dir entry
 *  @param address The address to set (must be page aligned)
 *  @param entry The entry to set the address in
 *  @return void
 **/
inline void set_entry_address(entry_t* entry, void* address)
{
    ASSERT_PAGE_ALIGNED(address);
    uint32_t addr = (uint32_t)address;
    entry->address = (addr >> ENTRY_ADDRESS_SHIFT);
}

/** @brief Casts a page aligned pointer to a page table
 *  @param address The address to cast to a page table
 *  @return The page table pointer
 **/
inline page_table_t* page_table(void* address)
{
    ASSERT_PAGE_ALIGNED(address);
    return (page_table_t*)address;
}

/** @brief Casts a page aligned pointer to a page directory
 *  @param address The address to cast to a page directory
 *  @return The page directory pointer
 **/
inline page_directory_t* page_directory(void* address)
{
    ASSERT_PAGE_ALIGNED(address);
    return (page_directory_t*)address;
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

/** @brief Gets the physical mapping of a virtual address given the page table
 *  @param cr2 The page table register
 *  @param virtual The virtual address
 *  @param physical The physical address
 *  @return zero on success, less than zero on error
 **/
int virtual_to_physical(void* cr2, void* virtual, void** physical)
{
    page_directory_t* dir = page_directory(cr2);
    entry_t* entry = get_dir_entry(virtual, dir);
    if (!entry->present) {
        return -1;
    }
    page_table_t* table = page_table(get_entry_address(*entry));
    entry = get_table_entry(virtual, table);
    if (!entry->present) {
        return -2;
    }
    void* address = get_address(virtual, get_entry_address(*entry));
    *physical = address;
    return 0;
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

/** @brief Copy a frame
 *  @param frame the frame to copy to
 *  @param from The frame to copy from
 **/
void copy_frame(void* frame, void* from)
{
    ASSERT_PAGE_ALIGNED(frame);
    ASSERT_PAGE_ALIGNED(from);
    memcpy(frame, from, PAGE_SIZE);
}

void map_physical(int table_idx, int pages, page_directory_t* dir, entry_t init)
{
    int page_idx;
    //get the address of the first page this page table will contain
    void* table_start = (void*)(table_idx * PAGE_SIZE * PAGES_PER_TABLE);
    //allocate a frame for the page table
    page_table_t* table = page_table(get_reserved_frame());
    //iterate over the physical pages which belong in this table
    for (page_idx = 0; page_idx < pages; page_idx++) {
        //add the page to the page table
        void* page = LEA(table_start, PAGE_SIZE, page_idx);
        table->pages[page_idx] = init;
        set_entry_address(&table->pages[page_idx], page);
    }
    //if there are extra entries in the page table, zero them
    for (page_idx = pages; page_idx < PAGES_PER_TABLE; page_idx++) {
        table->pages[page_idx] = (entry_t) { 0 };
    }
    //now add the page table to the page directory
    dir->tables[table_idx] = init;
    set_entry_address(&dir->tables[table_idx], table);
}

page_directory_t* init_kernel_vm()
{
    int page_tables = DIVIDE_ROUND_UP(machine_phys_frames(), PAGES_PER_TABLE);
    lprintf("%d page tables to be created", page_tables);
    //reserve frames for virutal memory
    if (reserve_frames(1 + page_tables) < 0) {
        lprintf("This really shouldn't fail");
    }
    //get frame for the page directory
    page_directory_t* directory = page_directory(get_reserved_frame());
    zero_frame(directory);
    entry_t kernel_table = {
        .present = 1,
        .write = 1,
        .global = 1,
    };
    entry_t user_table = {
        .present = 1,
        .write = 1,
    };
    int i;
    for (i = 0; i < USER_MEM_START / (PAGE_SIZE * PAGES_PER_TABLE); i++) {
        map_physical(i, PAGES_PER_TABLE, directory, kernel_table);
    }
    for (; i < machine_phys_frames() / PAGES_PER_TABLE; i++) {
        map_physical(i, PAGES_PER_TABLE, directory, user_table);
    }
    if (machine_phys_frames() % PAGES_PER_TABLE != 0) {
        int remaing_frames = machine_phys_frames() % PAGES_PER_TABLE;
        map_physical(i, remaing_frames, directory, user_table);
    }
    return directory;
}
