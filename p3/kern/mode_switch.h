/** @file mode_switch.h
 *
 *  @brief Interface for mode switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#ifndef _MODE_SWITCH_H
#define _MODE_SWITCH_H

#include <stdint.h>
#include <idt.h>
#include <control.h>

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

/** @brief Switches to user mode
 *
 *  @param esp Stack pointer with values to be restored
 *  @return Void
 **/
void first_entry_user_mode(void *esp);


// C headers
void set_esp0_wrapper(void *addr);


// Fault handlers
/** @brief Wrapper for the divide error handler
 *  @return void
 */
INT_ASM_H(IDT_DE);

/** @brief Wrapper for the debug exception handler
 *  @return void
 */
INT_ASM_H(IDT_DB);

/** @brief Wrapper for the NMI exception handler
 *  @return void
 */
INT_ASM_H(IDT_NMI);

/** @brief Wrapper for the breakpoint exception handler
 *  @return void
 */
INT_ASM_H(IDT_BP);

/** @brief Wrapper for the overflow exception handler
 *  @return void
 */
INT_ASM_H(IDT_OF);

/** @brief Wrapper for the BOUND exception handler
 *  @return void
 */
INT_ASM_H(IDT_BR);

/** @brief Wrapper for the invalid opcode exception handler
 *  @return void
 */
INT_ASM_H(IDT_UD);

/** @brief Wrapper for the no math coprocessor exception handler
 *  @return void
 */
INT_ASM_H(IDT_NM);

/** @brief Wrapper for the double fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_DF);

/** @brief Wrapper for the coprocessor segment overrun exception handler
 *  @return void
 */
INT_ASM_H(IDT_CSO);

/** @brief Wrapper for the invalid task segment selector exception handler
 *  @return void
 */
INT_ASM_H(IDT_TS);

/** @brief Wrapper for the segment not present exception handler
 *  @return void
 */
INT_ASM_H(IDT_NP);

/** @brief Wrapper for the stack segment fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_SS);

/** @brief Wrapper for the general protection exception handler
 *  @return void
 */
INT_ASM_H(IDT_GP);

/** @brief Wrapper for the page fault exception handler
 *  @return void
 */
INT_ASM_H(IDT_PF);

/** @brief Wrapper for the x87 FPU floating point error exception handler
 *  @return void
 */
INT_ASM_H(IDT_MF);

/** @brief Wrapper for the alignment check exception handler
 *  @return void
 */
INT_ASM_H(IDT_AC);

/** @brief Wrapper for the machine check exception handler
 *  @return void
 */
INT_ASM_H(IDT_MC);

/** @brief Wrapper for the SIMD floating point exception handler
 *  @return void
 */
INT_ASM_H(IDT_XF);
 
// Syscall handlers
/** @brief Wrapper for the gettid handler
 *  @return void
 */
NAME_ASM_H(gettid_syscall);

// Device interrupt handlers
/** @brief Wrapper for the timer interrupt handler
 *  @return void
 */
NAME_ASM_H(timer_interrupt);

/** @brief Wrapper for the keyboard interrupt handler
 *  @return void
 */
NAME_ASM_H(keyboard_interrupt);

#endif /* _MODE_SWITCH_H */
