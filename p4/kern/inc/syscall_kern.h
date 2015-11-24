/** @file syscall_kern.h
 *  @brief Interface for syscall related functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_SYSCALL_KERN_H
#define KERN_INC_SYSCALL_KERN_H

#include <control_block.h>
#include <ureg.h>
#include <stdint.h>

/** @brief Shuts down the system with a HLT instruction
 *  @return Does not return
 **/
void halt_asm();

void init_print();
void init_readline();
void register_swexn(tcb_t *tcb,swexn_handler_t handler,void *arg,void *stack);
void deregister_swexn(tcb_t *tcb);
void swexn_handler(ureg_t* state, tcb_t* tcb);

void init_timer();
int readline(int len, char *buf, tcb_t *tcb);

int getbytes( const char *filename, int offset, int size, char *buf );
void *create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);
tcb_t* new_program(char* fname, int argc, char** argv);

#endif // KERN_INC_SYSCALL_KERN_H
