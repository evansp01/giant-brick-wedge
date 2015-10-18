/** @file switch.h
 *  @brief Interface for context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SWITCH_H_
#define SWITCH_H_

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return Void
 **/
void create_context(void *stack);

/** @brief Restores the context to 
 *
 *  @param stack Stack pointer for the thread to be restored
 *  @return Void
 **/
void restore_context(void *stack);

/** @brief TODO: Fill this in
 *
 *  @return Void
 **/
void user_mode();


#endif // SWITCH_H_