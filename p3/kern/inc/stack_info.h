/** @file stack_info.h
 *  @brief Interface for stack related macros
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_STACK_INFO_H
#define KERN_INC_STACK_INFO_H

#include <page.h>
#include <stdint.h>

/** @brief Pushes values on to the stack pointer at the given address
 *
 *  @param stack The current stack address
 *  @param value The value to be pushed
 *  @param type The type of value pushed
 *  @return void
 **/
#define PUSH_STACK(stack, value, type)         \
    do {                                       \
        stack = (void*)(((type*)(stack)) - 1); \
        *((type*)(stack)) = (type)(value);     \
    } while (0)

/** @brief Number of bits to shift to get the stack size **/
#define K_STACK_SHIFT (PAGE_SHIFT + 1)
/** @brief Size of a kernel stack **/
#define K_STACK_SIZE (1 << K_STACK_SHIFT)
/** @brief Bitmask for address bits of a stack **/
#define K_STACK_ADDR_MASK ((uint32_t)K_STACK_SIZE - 1)
/** @brief Bitmask to find te stack base **/
#define K_STACK_BASE_MASK (~K_STACK_ADDR_MASK)
/** @brief Returns the space between the stack base and tcb pointer **/
#define K_STACK_SPACE (K_STACK_SIZE - 2*sizeof(int))
/** @brief Returns the stack top given the stack base **/
#define K_STACK_TOP(x) ((uint32_t)(x) + K_STACK_SPACE)
/** @brief Returns the stack base of a given stack address **/
#define K_STACK_BASE(x) ((uint32_t)(x) & K_STACK_BASE_MASK)

#endif /* KERN_INC_STACK_INFO_H */
