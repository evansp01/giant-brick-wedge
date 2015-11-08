
#ifndef _STACK_INFO_H
#define _STACK_INFO_H

#include <page.h>
#include <stdint.h>

#define PUSH_STACK(stack, value, type)         \
    do {                                       \
        stack = (void*)(((type*)(stack)) - 1); \
        *((type*)(stack)) = (type)(value);     \
    } while (0)

#define K_STACK_SHIFT PAGE_SHIFT
#define K_STACK_SIZE (1 << K_STACK_SHIFT)
#define K_STACK_ADDR_MASK ((uint32_t)K_STACK_SIZE - 1)
#define K_STACK_BASE_MASK (~K_STACK_ADDR_MASK)
#define K_STACK_SPACE (K_STACK_SIZE - 2*sizeof(int))
#define K_STACK_TOP(x) ((uint32_t)(x) + K_STACK_SPACE) 

#define K_STACK_BASE(x) ((uint32_t)(x) & K_STACK_BASE_MASK)
#define K_STACK_ADDR(x) ((uint32_t)(x) & K_STACK_ADDR_MASK)

#endif /* _STACK_INFO_H */
