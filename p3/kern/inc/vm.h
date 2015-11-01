#ifndef PAGE_STRUCTS_H_
#define PAGE_STRUCTS_H_

#include <stdint.h>
#include <page.h>
#include <stddef.h>

#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIR 1024
#define ENTRY_ADDRESS_SHIFT 12

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
    uint32_t unused : 3;        /* bit 9  - 11 */
    uint32_t address : 20;      /* bit 12 - 31 */
} entry_t;

typedef struct {
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

extern const entry_t e_kernel_dir;
extern const entry_t e_user_dir;
extern const entry_t e_kernel_global;
extern const entry_t e_kernel_local;
extern const entry_t e_read_page;
extern const entry_t e_write_page;

//page table manipulation
entry_t create_entry(void* address, entry_t model);
void* get_entry_address(entry_t entry);
void set_entry_address(entry_t* entry, void* address);
entry_t* get_dir_entry(void* address, page_directory_t* directory);
entry_t* get_table_entry(void* address, page_table_t* table);
void* get_address(void* address, void* page);
void turn_on_vm(page_directory_t* dir);
void zero_frame(void* frame);
void copy_frame(void* frame, void* from);
void init_virtual_memory();
page_directory_t* create_page_directory();
page_table_t* create_page_table();
page_directory_t* create_kernel_directory();
void free_page_directory(page_directory_t* dir);
int allocate_pages(void* cr2, void* start, size_t size, entry_t model);
int page_bytes_left(void* address);

//headers for frame alloc
void init_frame_alloc();
int reserve_frames(int frame_count);
void free_reserved_frames(int count);
void* get_reserved_frame();
void* allocate_frame();
void free_frame(void* frame);
void invalidate_page(void *page);
int vm_permissions(void* virtual, entry_t* permissions);
int vm_info(void* virtual, size_t size, entry_t* permissions);
int vm_addr_info(void* virtual, entry_t* permissions, void** physical);

int vm_set_readonly(void* cr3, void* start, int size);
int vm_set_readwrite(void* cr3, void* start, int size);
int vm_user_can_read(void* cr3, void* start, int size);
int vm_user_can_write(void* cr3, void* start, int size);
int vm_user_can_alloc(void* cr3, void* start, int size);
int vm_user_strlen(void* cr3, char* start);
int vm_user_arrlen(void* cr3, char** start);
int get_packet(void* packet, void* esi, size_t size);
uint32_t get_cr3();

#endif // PAGE_STRUCTS_H_
