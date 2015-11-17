/** @file swexn.c
 *
 *  @brief Functions to handle swexn syscalls
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/
 
#include <control.h>
#include <mode_switch.h>
#include <stdlib.h>
#include <syscall_kern.h>

/** @brief Registers a software exception handler
 *  @return void
 */
void register_swexn(tcb_t *tcb, swexn_handler_t handler, void *arg, void *stack)
{
    tcb->swexn.handler = handler;
    tcb->swexn.arg = arg;
    tcb->swexn.stack = stack;
}

/** @brief Deregisters the software exception handler
 *  @return void
 */
void deregister_swexn(tcb_t *tcb)
{
    tcb->swexn.handler = NULL;
    tcb->swexn.arg = NULL;
    tcb->swexn.stack = NULL;
}

/** @brief Swexn handler
 *
 *  @param state Struct containing saved register state before exception
 *  @param tcb TCB of current thread
 *  @return void
 **/
void swexn_handler(ureg_t* state, tcb_t* tcb)
{
    // Copy then deregister current software exception handler
    swexn_t swexn = tcb->swexn;
    deregister_swexn(tcb);
    
    // Setup exception stack
    swexn_stack_t swexn_stack;
    swexn_stack.ret_addr = 0;
    swexn_stack.arg = swexn.arg;
    swexn_stack.ureg = (void *)((uint32_t)swexn.stack - sizeof(ureg_t));
    swexn_stack.state = *state;
    void *start = (void *)((uint32_t)swexn.stack - sizeof(swexn_stack_t));
    vm_write(&tcb->process->directory, &swexn_stack, start, 
        sizeof(swexn_stack_t));
    
    // Setup context to switch to exception handler
    void *new_esp = create_context((uint32_t)tcb->kernel_stack,
        (uint32_t)start, (uint32_t)swexn.handler);
        
    // Run software exception handler
    go_to_user_mode(new_esp);
    
    // Should never reach here
    panic("We are lost in the depths of swexn");
}
