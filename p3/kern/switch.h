/** @file switch.h
 *  @brief Interface for context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SWITCH_H_
#define SWITCH_H_

#include <seg.h>
#include <stdint.h>

/** @brief Restores the context to
 *
 *  @param stack Stack pointer for the thread to be restored
 *  @return Void
 **/
void restore_context(void *stack);

#endif // SWITCH_H_
