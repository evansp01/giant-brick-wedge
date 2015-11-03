#include "vm_internal.h"

typedef struct {
    uint32_t present : 1;  /* bit 0 */
    uint32_t write : 1;    /* bit 1 */
    uint32_t user : 1;     /* bit 2 */
    uint32_t reserved : 1; /* bit 3 */
    uint32_t fetch : 1;    /* bit 4 */
    uint32_t junk : 27;    /* bit 5 - 31 */
} page_fault_t;

int reserved_or_fetch(page_fault_t error, uint32_t cr2)
{
    // instruction fetches can't be related to zfod
    if (error.fetch) {
        lprintf("Page fault caused by invalid instruction fetch at %lx", cr2);
        return -1;
    }
    // ideally we will never have these
    if (error.reserved) {
        lprintf("Page fault caused by reserved bit set to 1 at %lx", cr2);
        return -1;
    }
    return 0;
}

int perm_present(page_fault_t error, entry_t table, entry_t dir, uint32_t cr2)
{
    // User trying to access a kernel page, no thanks
    if (error.user && !is_user(&table, &dir)) {
        lprintf("User proc tried to access kernel memory at %lx", cr2);
        return -1;
    }
    // Whatever they want legitimately isn't there
    if (!table.present) {
        lprintf("Process tried to access not present page at %lx", cr2);
        return -1;
    }
    return 0;
}

uint32_t page_align(uint32_t address)
{
    return address & (~(PAGE_SIZE - 1));
}

int vm_resolve_pagefault(ppd_t* ppd, uint32_t cr2, int error_code)
{
    page_fault_t error = AS_TYPE(error_code, page_fault_t);
    // If things we don't deal with
    if (reserved_or_fetch(error, cr2) < 0) {
        return -1;
    }
    entry_t* table, *dir;
    // Page table or directory missing
    if (vm_get_address(ppd, (void*)cr2, &table, &dir) < 0) {
        lprintf("Getting the page table or dir failed at %lx", cr2);
        return -1;
    }
    // Check if user to kernel access or page not present in table
    if (perm_present(error, *table, *dir, cr2) < 0) {
        return -1;
    }
    // error { fetch = 0, reserved = 0}
    // table, dir { present = 1 }
    // error.user <= (table.user & dir.user)
    if (error.write <= table->write) {
        // note that the process may well have thought that the page wasnt
        // present, or was kernel only at the time of the fault
        lprintf("Access to %lx faulted, but seems to be cool now", cr2);
        return 0;
    }
    if (!table->zfod) {
        lprintf("Process tried to write to read only page at %lx", cr2);
        return -1;
    }
    // error { fetch = 0, reserved = 0}
    // table, dir { present = 1, zfod = 1 }
    // error.user <= (table.user & dir.user)
    // error.write > table.write
    if (error.write && !table->write) {
        uint32_t page = page_align(cr2);
        alloc_frame((void *)page, table, e_write_page);
        return 0;
    }
    // can this code path even happen?
    lprintf("I have no idea why this is a page fault at %lx", cr2);
    return -1;
}
