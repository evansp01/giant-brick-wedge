/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  Initializes the kernel.
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
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

/* additional includes */
#include <control.h>
#include <switch.h>
#include <common.h>
#include <vm.h>
#include <utilities.h>
#include <malloc.h>
#include <elf_410.h>
#include <string.h>
#include <eflags.h>
#include <setup_idt.h>
#include <kernel_tests.h>
#include <loader.h>

/** @brief Tick function, to be called by the timer interrupt handler
 * 
 *  @param ticks Number of ticks sent by timer
 *  @return Void
 **/
void timer(unsigned int ticks)
{
    return;
}

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t* mbinfo, int argc, char** argv, char** envp)
{
    install_exceptions();
    initialize_devices(timer);
    install_syscalls();
    init_frame_alloc();
    init_virtual_memory();
    
    enable_interrupts();
    
    page_directory_t* dir = create_kernel_directory();
    turn_on_vm(dir);
    init_kernel_state();
    
    // Run kernel tests
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
