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
#include <elf_410.h>
#include <string.h>
#include <eflags.h>
#include <setup_idt.h>


#define PAGE_SIZE_SQUARED PAGE_SIZE* PAGE_SIZE
#define NUM_INTEGERS 1345


void timer(unsigned int a) {

}

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
    install_exceptions();
    initialize_devices(timer);
    install_syscalls();
    init_frame_alloc();
    init_virtual_memory();
    enable_interrupts();
    MAGIC_BREAK;
    page_directory_t* dir = create_kernel_directory();
    turn_on_vm(dir);
    lprintf("virtual memory is enabled, and we haven't crashed");
    init_kernel_state();
    // 4. Enable interrupts
    vm_diagnose(dir);
    test_process_vm();

    if (create_idle() < 0) {
        panic("Cannot create first process. Kernel is sad");
    }

    while (1) {
        continue;
    }
    return 0;
}
