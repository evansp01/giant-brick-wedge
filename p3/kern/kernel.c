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
#include <common.h>
#include <page_structs.h>
#include <cr.h>

/** @brief Looks up some kinda random addresses to see if the identity map works
 *  @param page_directory The kernel page directory
 *  @return void
 **/
void vm_diagnose(void* page_directory)
{
    char* memory = 0;
    int i;
    for (i = 0; i < PAGE_SIZE * PAGE_SIZE * 5;
         i += PAGE_SIZE * PAGE_SIZE - 200) {
        void* physical;
        void* virtual = &memory[i];
        if (virtual_to_physical(page_directory, virtual, &physical) < 0) {
            lprintf("Error could not find mapping");
            continue;
        }
        if (virtual != physical) {
            lprintf("Error %x maps to %x", (unsigned int)virtual,
                    (unsigned int)physical);
        }
    }
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
    init_frame_alloc();
    // 1. Install fault handlers
    // 2. Setup VM
    page_directory_t* dir = init_kernel_vm();
    vm_diagnose(dir);
    set_cr3((uint32_t)dir);
    set_cr0(SET_BIT(get_cr0(), 31));
    // 3. Enable VM
    // 4. Enable interrupts
    lprintf("virtual memory is enabled, and we haven't crashed");

    while (1) {
        continue;
    }

    return 0;
}
