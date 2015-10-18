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

/** @brief Crafts the kernel stack for the initial program
 *
 *  @param stack Stack pointer for the thread stack to be crafted
 *  @return Void
 **/
void create_context(uint32_t stack, uint32_t user_esp, uint32_t user_eip)
{
    uint32_t *kernel_stack = (uint32_t *)stack;
    kernel_stack[0] = SEGSEL_USER_DS;
    kernel_stack[-1] = user_esp;
    kernel_stack[-2] = EFL_IOPL_RING0|EFL_IF|EFL_RESV1;
    kernel_stack[-3] = SEGSEL_USER_CS;
    kernel_stack[-4] = user_eip;
    kernel_stack[-5] = 0;
    // POPA
    kernel_stack[-6] = 0;   // EAX
    kernel_stack[-7] = 0;   // ECX
    kernel_stack[-8] = 0;   // EDX
    kernel_stack[-9] = 0;   // EBX
    kernel_stack[-10] = 0;  // ignored
    kernel_stack[-11] = 0;  // EBP
    kernel_stack[-12] = 0;  // ESI
    kernel_stack[-13] = 0;  // EDI
    // data segments
    kernel_stack[-14] = SEGSEL_USER_DS;  // DS
    kernel_stack[-15] = SEGSEL_USER_DS;  // ES
    kernel_stack[-16] = SEGSEL_USER_DS;  // FS
    kernel_stack[-17] = SEGSEL_USER_DS;  // GS
    
    user_mode_first(kernel_stack-17);
}