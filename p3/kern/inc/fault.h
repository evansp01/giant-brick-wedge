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
#include <ureg.h>

#define TRAP 0x7
#define INTERRUPT 0x6
#define KERNEL 0
#define USER 3

int handler_install();
void set_idt(void* handler, int segment, int privilege, int type, int index);
void keyboard_interrupt_asm();
void fault_handler(ureg_t state);
void timer_interrupt_asm();


#define _INT_ASM(number) interrupt##number##_asm
#define INT_ASM(number) _INT_ASM(number)
#define INT_ASM_H(number) void _INT_ASM(number)()

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


/** @def CONSTRUCT_HANDLER_H(NAME)
 *
 *  @brief Constructs the assembly wrapper function declaration
 *
 *  @param NAME Fault name
 *  @return Void
 **/
#define CONSTRUCT_HANDLER_H(NAME) \
    void NAME##_handler_asm()

/** @def CONSTRUCT_HANDLER_C(NAME)
 *
 *  @brief Constructs the C handler function name
 *
 *  @param NAME Fault name
 *  @return Void
 **/
#define CONSTRUCT_HANDLER_C(NAME) \
    void NAME##_handler()

// fault_asm.S headers
/** @brief Wrapper for the divide error handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(divide);

/** @brief Wrapper for the debug exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(debug);

/** @brief Wrapper for the NMI exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(nmi);

/** @brief Wrapper for the breakpoint exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(breakpoint);

/** @brief Wrapper for the overflow exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(overflow);

/** @brief Wrapper for the BOUND exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(bound);

/** @brief Wrapper for the invalid opcode exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(opcode);

/** @brief Wrapper for the no math coprocessor exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(no_math);

/** @brief Wrapper for the double fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(double_fault);

/** @brief Wrapper for the coprocessor segment overrun exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(cso);

/** @brief Wrapper for the invalid task segment selector exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(tss);

/** @brief Wrapper for the segment not present exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(not_present);

/** @brief Wrapper for the stack segment fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(stack);

/** @brief Wrapper for the general protection exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(protection);

/** @brief Wrapper for the page fault exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(page);

/** @brief Wrapper for the x87 FPU floating point error exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(math);

/** @brief Wrapper for the alignment check exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(alignment);

/** @brief Wrapper for the machine check exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(machine);

/** @brief Wrapper for the SIMD floating point exception handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(fpu);

/** @brief Wrapper for the gettid handler
 *  @return Void
 */
CONSTRUCT_HANDLER_H(gettid);

// fault.c headers

CONSTRUCT_HANDLER_C(gettid);

#endif /* _FAULT_H */
