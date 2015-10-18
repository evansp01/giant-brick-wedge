/** @file switch.h
 *  @brief Interface for context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SWITCH_H_
#define SWITCH_H_

#include <seg.h>

#ifdef ASSEMBLER

/**  @brief Saves state and switches to kernel mode
 **/
.macro KERNEL_MODE_SWITCH
    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    movl $SEGSEL_KERNEL_DS, %ds
    movl $SEGSEL_KERNEL_DS, %es
    movl $SEGSEL_KERNEL_DS, %fs
    movl $SEGSEL_KERNEL_DS, %gs
.endm

/**  @brief Restores state to user mode
 **/
.macro USER_MODE_SWITCH
    popl %gs
    popl %fs
    popl %es
    popl %ds
    popa
    iret
.endm

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
void user_mode_first(void *esp);

#endif


#endif // SWITCH_H_