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

#ifdef ASSEMBLER

/**  @brief Saves state and switches to kernel mode
 **/
#else

void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip);

/** @brief Restores the context to
 *
 *  @param stack Stack pointer for the thread to be restored
 *  @return Void
 **/
void restore_context(void *stack);

/** @brief TODO: Fill this in
 *
 *  @param esp Stack pointer with values to be restored
 *  @return Void
 **/
void user_mode_switch(void *esp);

#endif


#endif // SWITCH_H_
