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
#include <mode_switch.h>
#include <cr.h>
#include <scheduler.h>

/** @brief Tick function, to be called by the timer interrupt handler
 *
 *  @param ticks Number of ticks sent by timer
 *  @return Void
 **/
void timer(unsigned int ticks)
{
    return;
}

void init_malloc();

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
    init_virtual_memory();
    init_scheduler();
    init_kernel_state();

    // Run kernel tests (TODO: Free/reallocate frames)
    //vm_diagnose(dir);
    //test_process_vm();

    // Create 1st idle process
    tcb_t *tcb = new_program("shell", 0, NULL);
    if (tcb == NULL)
        panic("Cannot create first process. Kernel is sad");
    
    // Switch to multi-threaded mode
    init_malloc();

    // Switch to 1st idle thread
    // Interrupts cannot yet be enabled, as they will trigger a fault since
    // there is no pcb entry for this kernel stack
    // Interrupts will be enabled upon switching to user mode
    first_entry_user_mode(tcb->saved_esp);

    while (1) {
        panic("Kernel has wandered into limbo.");
    }
    return 0;
}
