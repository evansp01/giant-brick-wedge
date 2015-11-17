#ifndef H_VM_INTERNAL_
#define H_VM_INTERNAL_

#include <common_kern.h>
#include <utilities.h>
#include <stdint.h>
#include <page.h>
#include <stddef.h>
#include <mutex.h>

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024
#define ENTRY_ADDRESS_SHIFT 12
#define OVERCOMMIT_RATIO 1

#define KERNEL_TABLES DIVIDE_ROUND_UP(USER_MEM_START, PAGE_SIZE* PAGES_PER_TABLE)

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

typedef struct page_directory{
    entry_t tables[TABLES_PER_DIR];
} page_directory_t;

typedef struct {
    entry_t pages[PAGES_PER_TABLE];
} page_table_t;

typedef struct {
    uint32_t page_address : 12;     /* bits 0  - 11 */
    uint32_t page_table_index : 10; /* bits 12 - 21 */
    uint32_t page_dir_index : 10;   /* bits 22 - 31 */
} address_t;

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
 *  @param Page the page to invalidate
 *  @return void
 **/
void invalidate_page(void *page);

int add_alloc(ppd_t* ppd, void* start, uint32_t size);
int remove_alloc(ppd_t* ppd, void* start, uint32_t* size);

void release_frames(void* start, uint32_t size);
int reserve_frames(void* start, uint32_t size);

entry_t create_entry(void* address, entry_t model);
void* get_entry_address(entry_t entry);
void set_entry_address(entry_t* entry, void* address);
void print_entry(entry_t *entry);
entry_t* get_dir_entry(void* address, page_directory_t* directory);
entry_t* get_table_entry(void* address, page_table_t* table);
void* get_address(void* address, void* page);
void turn_on_vm();
void zero_frame(void* frame);
page_directory_t* alloc_page_directory();
page_directory_t* alloc_kernel_directory();
page_table_t* alloc_page_table();
void free_page_directory(page_directory_t* dir);
int allocate_pages(ppd_t *ppd, void* start, uint32_t size, entry_t model);
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

#endif //H_VM_INTERNAL_
