/** @file switch.c
 *  @brief Implementation of the context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <switch.h>
#include <stdint.h>
#include <seg.h>
#include <eflags.h>
#include <simics.h>
#include <cr.h>
#include <fault.h>

#define PUT_STACK(stack, value, type) \
    *((type*)(stack)) = (type)(value)

#define PUSH_STACK(stack, value, type)          \
    do {                                        \
        stack = (void *)(((type*)(stack)) - 1); \
        PUT_STACK(stack, value, type);          \
    } while (0)

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return Void
 **/
void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
{
    //set_esp-1(stack);
    void *kernel_stack = (void *)stack;
    PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t);
    //PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t);
    PUSH_STACK(kernel_stack, user_esp, uint32_t);
    PUSH_STACK(kernel_stack, EFL_RESV1, uint32_t);
    PUSH_STACK(kernel_stack, SEGSEL_USER_CS, uint32_t);
    PUSH_STACK(kernel_stack, user_eip, uint32_t);
    // POPA
    PUSH_STACK(kernel_stack, 0, uint32_t); // EAX
    PUSH_STACK(kernel_stack, 0, uint32_t); // ECX
    PUSH_STACK(kernel_stack, 0, uint32_t); // EDX
    PUSH_STACK(kernel_stack, 0, uint32_t); // EBX
    PUSH_STACK(kernel_stack, 0, uint32_t); // ignored
    PUSH_STACK(kernel_stack, 0, uint32_t); // EBP
    PUSH_STACK(kernel_stack, 0, uint32_t); // ESI
    PUSH_STACK(kernel_stack, 0, uint32_t); // EDI
    // data segments
    PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t); // DS
    PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t); // ES
    PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t); // FS
    PUSH_STACK(kernel_stack, SEGSEL_USER_DS, uint32_t); // GS

    MAGIC_BREAK;

    user_mode_first(kernel_stack);
}
