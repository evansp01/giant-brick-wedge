#include <vm.h>
#include <utilities.h>

typedef int (*vm_operator)(entry_t*, entry_t*, address_t, int);

int vm_map_pages(void* cr3, void* start, int size, vm_operator op)
{
    int i, j;
    page_directory_t* dir = cr3;
    char* end = ((char*)start) + size;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
    int value = 0;
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
            if (!table_entry->present) {
                lprintf("Page table not present");
                return -2;
            }
            if ((value = op(table_entry, dir_entry, location, value)) < 0) {
                return value;
            }
        }
    }
    return value;
}

int vm_user_can_alloc(void* cr3, void* start, int size)
{
    int i, j;
    page_directory_t* dir = cr3;
    char* end = ((char*)start) + size;
    address_t vm_start = AS_TYPE(start, address_t);
    address_t vm_end = AS_TYPE(end, address_t);
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

int vm_get_address(void* cr3, void* addr, entry_t* table, entry_t* dir)
{
    page_directory_t* page_dir = cr3;
    entry_t* dir_entry = get_dir_entry(addr, page_dir);
    if (!dir_entry->present) {
        return -1;
    }
    page_table_t* page_table = get_entry_address(*dir_entry);
    entry_t* table_entry = get_table_entry(addr, page_table);
    if (!table_entry->present) {
        return -1;
    }
    *table = *table_entry;
    *dir = *dir_entry;
    return page_bytes_left(addr);
}

int is_user(entry_t* table, entry_t* dir)
{
    return table->user && dir->user;
}

int vm_user_strlen(void* cr3, char* start)
{
    entry_t table, dir;
    int space = 0;
    int i, length;
    for (;;) {
        if ((length = vm_get_address(cr3, start+space, &table, &dir)) < 0) {
            return -1;
        }
        if (!is_user(&table, &dir)) {
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

int vm_user_arrlen(void* cr3, char** start)
{
    entry_t table, dir;
    int space = 0;
    int i, length;
    for (;;) {
        if ((length = vm_get_address(cr3, start+space, &table, &dir)) < 0) {
            return -1;
        }
        if (!is_user(&table, &dir)) {
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

int vm_user_write_h(entry_t* table, entry_t* dir, address_t addr, int status)
{

    if (is_user(table, dir) && table->write) {
        return 0;
    }
    return -3;
}

int vm_user_read_h(entry_t* table, entry_t* dir, address_t addr, int status)
{
    if (is_user(table, dir)) {
        return 0;
    }
    return -3;
}

int vm_set_readwrite_h(entry_t* table, entry_t* dir, address_t addr, int status)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (!table->write) {
        table->write = 1;
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

int vm_set_readonly_h(entry_t* table, entry_t* dir, address_t addr, int status)
{
    if (!is_user(table, dir)) {
        return -3;
    }
    if (table->write) {
        table->write = 0;
        invalidate_page(AS_TYPE(addr, void*));
    }
    return 0;
}

int vm_user_can_write(void* cr3, void* start, int size)
{
    return vm_map_pages(cr3, start, size, vm_user_write_h) == 0;
}

int vm_user_can_read(void* cr3, void* start, int size)
{
    return vm_map_pages(cr3, start, size, vm_user_read_h) == 0;
}

int vm_set_readwrite(void* cr3, void* start, int size)
{
    return vm_map_pages(cr3, start, size, vm_set_readwrite_h) == 0;
}

int vm_set_readonly(void* cr3, void* start, int size)
{
    return vm_map_pages(cr3, start, size, vm_set_readonly_h) == 0;
}
