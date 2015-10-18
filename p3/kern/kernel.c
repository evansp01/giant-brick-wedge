/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h> /* lprintf() */

/* multiboot header file */
#include <multiboot.h> /* boot_info */

/* x86 specific includes */
#include <x86/asm.h> /* enable_interrupts() */
#include <control.h>
#include <switch.h>
#include <common.h>
#include <vm.h>
#include <cr.h>
#include <utilities.h>
#include <malloc.h>
#include <fault.h>
#include <elf_410.h>
#include <string.h>

#define PAGE_SIZE_SQUARED PAGE_SIZE* PAGE_SIZE
#define NUM_INTEGERS 1345

void test_process_vm()
{
    int i, j;
    char* memory = 0;
    page_directory_t* dir = create_page_directory();

    int* reference = malloc(NUM_INTEGERS * sizeof(int));
    int* copy = malloc(NUM_INTEGERS * sizeof(int));

    //set up reference with some values
    for (i = 0; i < NUM_INTEGERS; i++) {
        reference[i] = i + 1;
        copy[i] = 0;
    }

    for (i = 0; i < PAGE_SIZE_SQUARED * 10; i += PAGE_SIZE_SQUARED - 200) {
        void* virtual = &memory[i];
        vm_write(dir, virtual, reference, NUM_INTEGERS * sizeof(int));
        vm_read(dir, virtual, copy, NUM_INTEGERS * sizeof(int));

        //check to see read and write worked
        int failed = 0;
        for (j = 0; j < NUM_INTEGERS; j++) {
            if (copy[j] != reference[j]) {
                lprintf("read write error at %d %d vd %d", j, copy[j], reference[j]);
                failed = 1;
                break;
            }
        }
        if (!failed) {
            lprintf("passed test writing to %x", i);
        }

        //reset copy
        for (j = 0; j < NUM_INTEGERS; j++) {
            copy[j] = 0;
        }
    }
}

/** @brief Looks up some kinda random addresses to see if the identity map works
 *  @param page_directory The kernel page directory
 *  @return void
 **/
void vm_diagnose(void* page_directory)
{
    char* memory = 0;
    int i;
    for (i = 0; i < PAGE_SIZE_SQUARED * 12; i += PAGE_SIZE_SQUARED - 200) {
        void* physical;
        void* virtual = &memory[i];
        if (vm_to_physical(page_directory, virtual, &physical, NULL) < 0) {
            lprintf("Error could not find mapping");
            continue;
        }
        if (virtual != physical) {
            lprintf("Error %x maps to %x", (unsigned int)virtual,
                    (unsigned int)physical);
        }
    }
}

void turn_on_vm(page_directory_t* dir)
{
    set_cr3((uint32_t)dir);
    set_cr4(get_cr4() | CR4_PGE);
    set_cr0(get_cr0() | CR0_PG);
}

int setup_proc_address(pcb_t *pcb, char* procname)
{
    if (elf_check_header(procname) < 0) {
        lprintf("%s not a process", procname);
        return -1;
    }
    simple_elf_t elf;
    elf_load_helper(&elf, procname);
    page_directory_t* dir = create_page_directory();
    set_cr3((uint32_t)dir);
    pcb->directory = dir;

    allocate_pages(dir, (void *)elf.e_txtstart, elf.e_txtlen, e_read_page);
    getbytes(procname, elf.e_txtoff, elf.e_txtlen, (char*)elf.e_txtstart);

    allocate_pages(dir, (void *)elf.e_rodatstart, elf.e_rodatlen, e_read_page);
    getbytes(procname, elf.e_rodatoff, elf.e_rodatlen, (char*)elf.e_rodatstart);

    allocate_pages(dir, (void *)elf.e_datstart, elf.e_datlen, e_write_page);
    getbytes(procname, elf.e_datoff, elf.e_datlen, (char*)elf.e_datstart);

    allocate_pages(dir, (void *)elf.e_bssstart, elf.e_bsslen, e_write_page);
    memset((void*)elf.e_bssstart, 0, elf.e_bsslen);
    return 0;
}

int create_idle()
{
    init_kernel_state();
    pcb_t *pcb_entry = create_pcb_entry(NULL);
    
    void *stack = allocate_kernel_stack();
    if (stack == NULL)
        return -1;
    
    tcb_t *tcb_entry = create_tcb_entry(pcb_entry, stack);
    
    return load_program(pcb_entry, tcb_entry, "idle");
}

int load_program(pcb_t *pcb, tcb_t *tcb, char *filename)
{
    setup_proc_address(pcb, "idle");
    
    // Craft kernel stack contents
    //create_context(tcb->kernel_stack);
    
    restore_context(tcb->kernel_stack);
    
    return 0;
}

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t* mbinfo, int argc, char** argv, char** envp)
{
    /*
     * When kernel_main() begins, interrupts are DISABLED.
     * You should delete this comment, and enable them --
     * when you are ready.
     */

    lprintf("Hello from a brand new kernel!");
    // 1. Install fault handlers
    handler_install();
    init_frame_alloc();
    init_virtual_memory();
    page_directory_t* dir = create_kernel_directory();
    turn_on_vm(dir);
    // 4. Enable interrupts
    lprintf("virtual memory is enabled, and we haven't crashed");
    vm_diagnose(dir);
    test_process_vm();
    char* yolo = (char*)0xf0000000;
    *yolo = 4;
    
    create_idle();
    
    while (1) {
        continue;
    }
    return 0;
}
