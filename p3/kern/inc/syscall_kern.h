/** @file syscall.h
 *  @brief Interface for syscall related functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SYSCALL_KERN_H_
#define SYSCALL_KERN_H_

#include <control.h>
#include <ureg.h>

void init_syscalls();
void halt_asm();
void register_swexn(tcb_t *tcb,swexn_handler_t handler,void *arg,void *stack);
void deregister_swexn(tcb_t *tcb);
void swexn_handler(ureg_t* state, tcb_t* tcb);

#endif // SYSCALL_KERN_H_