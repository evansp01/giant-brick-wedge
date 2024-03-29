/** @file switch.h
 *  @brief Interface for context switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_SWITCH_H
#define KERN_INC_SWITCH_H

#include <seg.h>
#include <stdint.h>
#include <control_block.h>

/** @brief Restores the context to another kernel stack
 *
 *  @param stack Stack pointer for the thread to be restored
 *  @param tcb TCB of current thread to save esp
 *  @return Void
 **/
void switch_stack_and_regs(void *stack, tcb_t *tcb);

// C functions
void store_esp(void *saved_esp, tcb_t *tcb);
void context_switch(tcb_t *from, tcb_t *to);
void setup_for_switch(tcb_t* tcb);

#endif // KERN_INC_SWITCH_H
