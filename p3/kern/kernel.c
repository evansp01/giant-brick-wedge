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
#include <vm.h>
#include <malloc.h>
#include <elf_410.h>
#include <string.h>
#include <eflags.h>
#include <setup_idt.h>
#include <mode_switch.h>
#include <scheduler.h>
#include <switch.h>
#include <syscall_kern.h>

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
    init_timer();
    install_devices();
    init_print();
    init_readline();
    install_syscalls();
    init_virtual_memory();
    init_kernel_state();
    // Create idle process
    tcb_t *idle = new_program("idle", 0, NULL);
    // Allow for correct context switching to idle
    setup_for_switch(idle);
    // Create main program kernel will run
    tcb_t *tcb = new_program("init", 0, NULL);
    kernel_state.init = tcb;
    init_scheduler(idle, tcb);
    // Switch to thread safe malloc
    // this **MUST** be done after all other initialization has been performed
    // otherwise semaphores can randomly enable interrupts
    init_malloc();
    init_mutexes();
    //  Switch to ppd of first thread
    // Switch to 1st idle thread
    // Interrupts cannot yet be enabled, as they will trigger a fault since
    // there is no pcb entry for this kernel stack
    // Interrupts will be enabled upon switching to user mode
    go_to_user_mode(tcb->saved_esp);

    while (1) {
        panic("Kernel has wandered into limbo.");
    }
    return 0;
}
