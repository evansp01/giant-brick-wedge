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
#include <stdint.h>

void init_syscalls();
void halt_asm();
void register_swexn(tcb_t *tcb,swexn_handler_t handler,void *arg,void *stack);
void deregister_swexn(tcb_t *tcb);
void swexn_handler(ureg_t* state, tcb_t* tcb);

void init_timer();
int readline(int len, char *buf, tcb_t *tcb, ppd_t *ppd);

int getbytes( const char *filename, int offset, int size, char *buf );
void *create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);
tcb_t* new_program(char* fname, int argc, char** argv);

#endif // SYSCALL_KERN_H_
