/** @file fault_asm.S
 *  @brief Fault handler wrappers written in assembly
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 */

#include <switch.h>

.macro CONSTRUCT_HANDLER NAME
    .global \NAME\()_handler_asm
    \NAME\()_handler_asm:
        KERNEL_MODE_SWITCH
        call \NAME\()_handler
        USER_MODE_SWITCH
.endm


// Assembly fault handler wrapper functions

CONSTRUCT_HANDLER divide
CONSTRUCT_HANDLER debug
CONSTRUCT_HANDLER nmi
CONSTRUCT_HANDLER breakpoint
CONSTRUCT_HANDLER overflow
CONSTRUCT_HANDLER bound
CONSTRUCT_HANDLER opcode
CONSTRUCT_HANDLER no_math
CONSTRUCT_HANDLER double_fault
CONSTRUCT_HANDLER cso
CONSTRUCT_HANDLER tss
CONSTRUCT_HANDLER not_present
CONSTRUCT_HANDLER stack
CONSTRUCT_HANDLER protection
CONSTRUCT_HANDLER page
CONSTRUCT_HANDLER math
CONSTRUCT_HANDLER alignment
CONSTRUCT_HANDLER machine
CONSTRUCT_HANDLER fpu


