/** @file switch.S
 *  @brief Implementation of the context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <switch.h>

.global restore_context
restore_context:
    movl 4(%esp), %eax      # save the stack pointer argument
    movl %eax, %esp         # switch stack pointers
    popa                    # restore general purpose registers


