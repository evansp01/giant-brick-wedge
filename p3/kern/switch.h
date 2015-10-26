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

/** @brief Switches context to another thread
 *
 *  @param tid Thread id of thread to yield to, if -1 then yield to any thread
 *  @return Void
 **/
int yield(void *addr, int yield_tid);

#endif // SWITCH_H_
