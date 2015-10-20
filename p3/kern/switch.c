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
#include <ureg.h>
#include <mode_switch.h>

#define PUT_STACK(stack, value, type) \
    *((type*)(stack)) = (type)(value)

#define PUSH_STACK(stack, value, type)         \
    do {                                       \
        stack = (void*)(((type*)(stack)) - 1); \
        PUT_STACK(stack, value, type);         \
    } while (0)

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return void
 **/
void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
{
    //set_esp-1(stack);
    uint32_t eflags_start = EFL_RESV1 | EFL_IF;
    ureg_t ureg = {
        .ss = SEGSEL_USER_DS,
        .esp = user_esp,
        .eflags = eflags_start,
        .cs = SEGSEL_USER_CS,
        .eip = user_eip,
        .gs = SEGSEL_USER_DS,
        .fs = SEGSEL_USER_DS,
        .es = SEGSEL_USER_DS,
        .ds = SEGSEL_USER_DS
    };
    void* kernel_stack = (void*)stack;
    PUSH_STACK(kernel_stack, ureg, ureg_t);
    user_mode_switch(kernel_stack);
}
