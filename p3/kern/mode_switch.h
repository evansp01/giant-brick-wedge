/** @file fault.h
 *
 *  @brief Function header for fault handlers
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef _FAULT_H
#define _FAULT_H

#include <stdint.h>
#include <idt.h>

#define _NAME_ASM(name) name##_asm
/** @def CONSTRUCT_HANDLER_H(NAME)
 *
 *  @brief Constructs the assembly wrapper function declaration
 *
 *  @param NAME Fault name
 *  @return void
 **/
#define NAME_ASM_H(name) void _NAME_ASM(name)()

/** @def CONSTRUCT_HANDLER_C(NAME)
 *
 *  @brief Constructs the C handler function name
 *
 *  @param NAME Fault name
 *  @return void
 **/
#define NAME_ASM(name) _NAME_ASM(name)


#define _INT_ASM(number) interrupt##number##_asm
#define INT_ASM(number) _INT_ASM(number)
#define INT_ASM_H(number) void _INT_ASM(number)()

void user_mode_switch(void *esp);

NAME_ASM_H(gettid_syscall);
NAME_ASM_H(timer_interrupt);
NAME_ASM_H(keyboard_interrupt);

INT_ASM_H(IDT_DE);
INT_ASM_H(IDT_DB);
INT_ASM_H(IDT_NMI);
INT_ASM_H(IDT_BP);
INT_ASM_H(IDT_OF);
INT_ASM_H(IDT_BR);
INT_ASM_H(IDT_UD);
INT_ASM_H(IDT_NM);
INT_ASM_H(IDT_DF);
INT_ASM_H(IDT_CSO);
INT_ASM_H(IDT_TS);
INT_ASM_H(IDT_NP);
INT_ASM_H(IDT_SS);
INT_ASM_H(IDT_GP);
INT_ASM_H(IDT_PF);
INT_ASM_H(IDT_MF);
INT_ASM_H(IDT_AC);
INT_ASM_H(IDT_MC);
INT_ASM_H(IDT_XF);




// fault_asm.S headers
/** @brief Wrapper for the divide error handler
 *  @return void
 */

/** @brief Wrapper for the debug exception handler
 *  @return void
 */

/** @brief Wrapper for the NMI exception handler
 *  @return void
 */

/** @brief Wrapper for the breakpoint exception handler
 *  @return void
 */

/** @brief Wrapper for the overflow exception handler
 *  @return void
 */

/** @brief Wrapper for the BOUND exception handler
 *  @return void
 */

/** @brief Wrapper for the invalid opcode exception handler
 *  @return void
 */

/** @brief Wrapper for the no math coprocessor exception handler
 *  @return void
 */

/** @brief Wrapper for the double fault exception handler
 *  @return void
 */

/** @brief Wrapper for the coprocessor segment overrun exception handler
 *  @return void
 */

/** @brief Wrapper for the invalid task segment selector exception handler
 *  @return void
 */

/** @brief Wrapper for the segment not present exception handler
 *  @return void
 */

/** @brief Wrapper for the stack segment fault exception handler
 *  @return void
 */

/** @brief Wrapper for the general protection exception handler
 *  @return void
 */

/** @brief Wrapper for the page fault exception handler
 *  @return void
 */

/** @brief Wrapper for the x87 FPU floating point error exception handler
 *  @return void
 */

/** @brief Wrapper for the alignment check exception handler
 *  @return void
 */

/** @brief Wrapper for the machine check exception handler
 *  @return void
 */

/** @brief Wrapper for the SIMD floating point exception handler
 *  @return void
 */

/** @brief Wrapper for the gettid handler
 *  @return void
 */

// fault.c headers

#endif /* _FAULT_H */
