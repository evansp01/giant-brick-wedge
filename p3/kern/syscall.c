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
    
    // TODO: Copy software exception handler (if installed)
    
    
    // Return child tid to parent
    state.eax = tcb_child->id;
}
