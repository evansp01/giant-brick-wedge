/** @file common.h
 *  @brief (Temporary) Interface for fileless functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef COMMON_H_
#define COMMON_H_

#include <control.h>
#include <ureg.h>

//fault handler stuff
int init_timer();
void init_syscalls();
int readline(int len, char *buf, tcb_t *tcb);

void register_swexn(tcb_t *tcb,swexn_handler_t handler,void *arg,void *stack);
void deregister_swexn(tcb_t *tcb);
void swexn_handler(ureg_t* state, tcb_t* tcb);

#endif // COMMON_H_
