/** @file switch.h
 *  @brief Interface for context switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SWITCH_H_
#define SWITCH_H_

#include <seg.h>
#include <stdint.h>
#include <control.h>

/** @brief Restores the context to another kernel stack
 *
 *  @param stack Stack pointer for the thread to be restored
 *  @param tcb TCB of current thread to save esp
 *  @return Void
 **/
void switch_context(void *stack, tcb_t *tcb);

/** @brief Get the current value of esp
 *  @return The value of esp
 **/
uint32_t get_esp();

// C functions
int yield(int yield_tid);
void store_esp(void *saved_esp, tcb_t *tcb);


#endif // SWITCH_H_