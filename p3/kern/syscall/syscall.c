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

/** @brief Handler function for gettid()
 *
 *  @return void
 */
void gettid_syscall(ureg_t state)
{
    tcb_t *p_tcb = get_tcb();
    // return the tid
    state.eax = p_tcb->id;
}

/** @brief Handler function for fork()
 *
 *  @return void
 */
void fork_syscall(ureg_t state)
{
    tcb_t *tcb_parent = get_tcb();
    
    // Copy memory regions to new memory
    tcb_t *tcb_child = create_copy(tcb_parent);
    
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

/** @brief Handler function for set_status()
 *
 *  @return void
 */
void set_status_syscall(ureg_t state)
{
    tcb_t *tcb = get_tcb();
    pcb_t *pcb = tcb->parent;
    pcb->exit_status = state.esi;
}

/** @brief Handler function for vanish()
 *
 *  @return void
 */
void vanish_syscall(ureg_t state)
{
    //tcb_t *p_tcb = get_tcb();
    
    // TODO: Vanish syscall
    lprintf("Vanish syscall handler running");
    while (1) {
    }
}

/** @brief Handler function for wait()
 *
 *  @return void
 */
void wait_syscall(ureg_t state)
{
    //tcb_t *p_tcb = get_tcb();
    
    // TODO: Wait syscall
    lprintf("Wait syscall handler running");
    while (1) {
    }
}

/** @brief Handler function for halt()
 *
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
