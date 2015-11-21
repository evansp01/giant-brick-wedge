/** @file vm_internal.h
 *  @brief Interface for internally used VM functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef H_VM_INTERNAL
#define H_VM_INTERNAL

#include <common_kern.h>
#include <stdint.h>
#include <page.h>
#include <stddef.h>
#include <mutex.h>
#include "debug_assert.h"

/** @brief The number of page table entries per page table */
#define PAGES_PER_TABLE 1024
/** @brief The number of page directory entries per page directory */
#define TABLES_PER_DIR 1024
/** @brief The shift which gives the address bits of a table entry */
#define ENTRY_ADDRESS_SHIFT PAGE_SHIFT
/** @brief The ratio to overcommit virtual frames at 1 does not overcommit */
#define OVERCOMMIT_RATIO 1

/** @brief Divide x and y rounding the result up to the nearest integer */
#define DIVIDE_ROUND_UP(x, y) (1 + ((x) - 1) / (y))

/** @brief The number of page tables used by the kernel address space */
#define KERNEL_TABLES \
    DIVIDE_ROUND_UP(USER_MEM_START, PAGE_SIZE* PAGES_PER_TABLE)

/** @def AS_TYPE(address, type)
 *
 *  @brief Casts an address to a type with no type checking
 *  @param address The address to cast
 *  @param type The type to cast it to
 *  @return The address as a type
 **/
#define AS_TYPE(address, type) (*(type*)&(address))

/** @brief Struct for page directory/table entries */
typedef struct {
    uint32_t present : 1;       /* bit 0 */
    uint32_t write : 1;         /* bit 1 */
    uint32_t user : 1;          /* bit 2 */
    uint32_t write_through : 1; /* bit 3 */
    uint32_t cache_disable : 1; /* bit 4 */
    uint32_t accessed : 1;      /* bit 5 */
    uint32_t dirty : 1;         /* bit 6 */
    uint32_t page_size : 1;     /* bit 7 */
    uint32_t global : 1;        /* bit 8 */
    uint32_t zfod : 1;          /* bit 9 */
    uint32_t unused : 2;        /* bit 10  - 11 */
    uint32_t address : 20;      /* bit 12 - 31 */
} entry_t;

/** @brief Struct for a page directory */
typedef struct page_directory{
    entry_t tables[TABLES_PER_DIR];
} page_directory_t;

/** @brief Struct for a page table */
typedef struct {
    entry_t pages[PAGES_PER_TABLE];
} page_table_t;

/** @brief Struct for addresses */
typedef struct {
    uint32_t page_address : 12;     /* bits 0  - 11 */
    uint32_t page_table_index : 10; /* bits 12 - 21 */
    uint32_t page_dir_index : 10;   /* bits 22 - 31 */
} address_t;

/** @brief Global struct for virtual memory */
struct virtual_memory{
    page_directory_t *identity;
    page_table_t* kernel_pages[KERNEL_TABLES];
    int available_frames;
    mutex_t lock;
};

extern struct virtual_memory virtual_memory;

extern const entry_t e_kernel_dir;
extern const entry_t e_user_dir;
extern const entry_t e_kernel_global;
extern const entry_t e_kernel_local;
extern const entry_t e_read_page;
extern const entry_t e_write_page;
extern const entry_t e_zfod_page;
extern const entry_t e_unmapped;

/** @brief Invalidates a page using the invl page instruction
 *  @param page the page to invalidate
 *  @return void
 **/
void invalidate_page(void *page);

int add_alloc(ppd_t* ppd, void* start, uint32_t size);

void release_frames(void* start, uint32_t size);
int reserve_frames(void* start, uint32_t size);

entry_t create_entry(void* address, entry_t model);
void* get_entry_address(entry_t entry);
entry_t* get_dir_entry(void* address, page_directory_t* directory);
entry_t* get_table_entry(void* address, page_table_t* table);
void* get_address(void* address, void* page);
void zero_frame(void* frame);
page_directory_t* alloc_page_directory();
page_directory_t* alloc_kernel_directory();
page_table_t* alloc_page_table();
int page_bytes_left(void* address);
int copy_page_dir(page_directory_t* dir_child, page_directory_t* dir_parent);

int vm_free_alloc(ppd_t* ppd, uint32_t start, uint32_t size);

int is_user(entry_t* table, entry_t* dir);
int is_write(entry_t* table);
int is_zfod(entry_t* table);
int is_present_user(entry_t* entry);
int vm_get_address(ppd_t* ppd, void* addr, entry_t** table, entry_t** dir);

void init_frame_alloc();
void* get_zero_page();
int user_frame_total();
int alloc_frame(void* virtual, entry_t* table, entry_t model);
int kernel_alloc_frame(entry_t* table, entry_t model);
void free_frame(void* virtual, void *physical);

int allocate_tables(ppd_t* ppd, void* start, uint32_t size);

#endif //H_VM_INTERNAL
