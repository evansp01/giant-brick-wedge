/** @file syscall.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <simics.h>
#include <ureg.h>
#include <stdint.h>
#include <common_kern.h>
#include <control.h>
#include <loader.h>
#include <scheduler.h>
#include <stdlib.h>
#include <string.h>

/** @brief Handler function for fork()
 *
 *  @return void
 */
void fork_syscall(ureg_t state)
{
    tcb_t* tcb_parent = get_tcb();

    // Copy memory regions to new memory
    tcb_t* tcb_child = create_copy(tcb_parent);

    // Copy kernel stack with return value of 0 for child
    state.eax = 0;
    copy_kernel_stack(tcb_parent, tcb_child);

    // Setup stack for re-entry via context_switch
    setup_for_switch(tcb_child);

    // Schedule the child
    schedule(tcb_child);

    // TODO: Copy software exception handler (if installed)

    // Return child tid to parent
    state.eax = tcb_child->id;
}

/** @brief The set_status syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_status_syscall(ureg_t state)
{
    tcb_t *tcb = get_tcb();
    pcb_t *pcb = tcb->parent;
    pcb->exit_status = state.esi;
}

/** @brief The vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called vanish. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The task_vanish syscall
 *  @param state The current state in user mode
 *  @return void
 */
void task_vanish_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called task_vanish. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The wait syscall
 *  @param state The current state in user mode
 *  @return void
 */
void wait_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called wait. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The gettid syscall
 *  @param state The current state in user mode
 *  @return void
 */
void gettid_syscall(ureg_t state)
{
    tcb_t* p_tcb = get_tcb();
    // return the tid
    state.eax = p_tcb->id;
}

/** @brief The yield syscall
 *  @param state The current state in user mode
 *  @return void
 */
void yield_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called yield. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The deschedule syscall
 *  @param state The current state in user mode
 *  @return void
 */
void deschedule_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called deschedule. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The make_runnable syscall
 *  @param state The current state in user mode
 *  @return void
 */
void make_runnable_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called make_runnable. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The get_ticks syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_ticks_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called get_ticks. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The sleep syscall
 *  @param state The current state in user mode
 *  @return void
 */
void sleep_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called sleep. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The new_pages syscall
 *  @param state The current state in user mode
 *  @return void
 */
void new_pages_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called new_pages. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The remove_pages syscall
 *  @param state The current state in user mode
 *  @return void
 */
void remove_pages_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called remove_pages. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The getchar syscall
 *  @param state The current state in user mode
 *  @return void
 */
void getchar_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called getchar. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The readline syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readline_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called readline. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The print syscall
 *  @param state The current state in user mode
 *  @return void
 */
void print_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called print. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The set_term_color syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_term_color_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called set_term_color. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The set_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void set_cursor_pos_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called set_cursor_pos. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The get_cursor_pos syscall
 *  @param state The current state in user mode
 *  @return void
 */
void get_cursor_pos_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called get_cursor_pos. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The halt syscall
 *  @param state The current state in user mode
 *  @return void
 */
void halt_syscall(ureg_t state)
{
    // Halt machines running on simics
    sim_halt();
    // Halt machines running on real hardware
    panic("Running on real hardware, please add code to initiate shutdown\
           based on current hardware implementation!");
}


/** @brief The readfile syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readfile_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called readfile. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The misbehave syscall
 *  @param state The current state in user mode
 *  @return void
 */
void misbehave_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called misbehave. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}

/** @brief The swexn syscall
 *  @param state The current state in user mode
 *  @return void
 */
void swexn_syscall(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    lprintf("Thread %d called swexn. Not yet implemented", tcb->id);
    while(1) {
        continue;
    }
}
