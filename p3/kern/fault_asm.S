/** @file fault_asm.S
 *  @brief Fault handler wrappers written in assembly
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 */

#include <switch.h>
#include <idt.h>

.macro SYSCALL_HANDLER NAME
    .global \NAME\()_handler_asm
    \NAME\()_handler_asm:
        KERNEL_MODE_SWITCH
        call \NAME\()_handler
        USER_MODE_SWITCH
        iret
.endm

.macro INTERRUPT_WRAPPER_ASM NAME
    .global \NAME\()_asm
    \NAME\()_asm:
        KERNEL_MODE_SWITCH
        call \NAME\()
        USER_MODE_SWITCH
        iret
.endm

INTERRUPT_WRAPPER_ASM timer_interrupt
INTERRUPT_WRAPPER_ASM keyboard_interrupt

SYSCALL_HANDLER gettid



.macro INTERRUPT_ASM_ERROR INT
    .global interrupt\INT\()_asm
    interrupt\INT\()_asm:
        KERNEL_MODE_SWITCH
        pushl $0
        pushl $\INT\()
        call fault_handler
        add $8, %esp
        USER_MODE_SWITCH
        add $4, %esp
        iret
.endm

.macro INTERRUPT_ASM INT
    .global interrupt\INT\()_asm
    interrupt\INT\()_asm:
        pushl $0
        KERNEL_MODE_SWITCH
        pushl $0
        pushl $\INT\()
        call fault_handler
        add $8, %esp
        USER_MODE_SWITCH
        add $4, %esp
        iret
.endm

INTERRUPT_ASM IDT_DE
INTERRUPT_ASM IDT_DB
INTERRUPT_ASM IDT_NMI
INTERRUPT_ASM IDT_BP
INTERRUPT_ASM IDT_OF
INTERRUPT_ASM IDT_BR
INTERRUPT_ASM IDT_UD
INTERRUPT_ASM IDT_NM
INTERRUPT_ASM_ERROR IDT_DF
INTERRUPT_ASM IDT_CSO
INTERRUPT_ASM_ERROR IDT_TS
INTERRUPT_ASM_ERROR IDT_NP
INTERRUPT_ASM_ERROR IDT_SS
INTERRUPT_ASM_ERROR IDT_GP
INTERRUPT_ASM_ERROR IDT_PF
INTERRUPT_ASM IDT_MF
INTERRUPT_ASM_ERROR IDT_AC
INTERRUPT_ASM IDT_MC
INTERRUPT_ASM IDT_XF


