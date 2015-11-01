#ifndef H_VM_INTERNAL_
#define H_VM_INTERNAL_

#include <common_kern.h>
#include <utilities.h>

#define KERNEL_TABLES DIVIDE_ROUND_UP(USER_MEM_START, PAGE_SIZE* PAGES_PER_TABLE)

void invalidate_page(void *page);

struct virtual_memory{
    page_directory_t *identity;
    page_table_t* kernel_pages[KERNEL_TABLES];
};

extern struct virtual_memory virtual_memory;


#endif //H_VM_INTERNAL_
