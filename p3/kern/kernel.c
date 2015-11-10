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
#include <scheduler.h>
#include <switch.h>

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
    init_kernel_state();
    // Create idle process
    tcb_t *idle = new_program("idle", 0, NULL);
    if (idle == NULL) {
        panic("Cannot create first process. Kernel is sad");
    }
    // Allow for correct context switching to idle
    setup_for_switch(idle);
    // Create main program kernel will run
    tcb_t *tcb = new_program("fork_exit_bomb", 0, NULL);
    if (tcb == NULL) {
        panic("Cannot create first process. Kernel is sad");
    }
    init_scheduler(idle, tcb);
    // Switch to thread safe malloc
    // this **MUST** be done after all other initialization has been performed
    // otherwise semaphores can randomly enable interrupts
    init_malloc();
    //  Switch to ppd of first thread
    // Switch to 1st idle thread
    // Interrupts cannot yet be enabled, as they will trigger a fault since
    // there is no pcb entry for this kernel stack
    // Interrupts will be enabled upon switching to user mode
    scheduler_pre_switch(NULL, tcb);
    go_to_user_mode(tcb->saved_esp);

    while (1) {
        panic("Kernel has wandered into limbo.");
    }
    return 0;
}
