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
#include <control_block.h>

/** @brief A helper macro which expands tokens given to it for NAME_ASM */
#define _NAME_ASM(name) name##_asm

/** @def NAME_ASM_H(name)
 *
 *  @brief Constructs the assembly wrapper function declaration
 *
 *  @param name Fault name
 *  @return void
 **/
#define NAME_ASM_H(name) void _NAME_ASM(name)()

/** @def NAME_ASM(name)
 *
 *  @brief Constructs the assembly handler name for a c function
 *
 *  @param name The name of the c function
 *  @return void
 **/
#define NAME_ASM(name) _NAME_ASM(name)

/** @brief A helper macro for INT_ASM */
#define _INT_ASM(number) interrupt##number##_asm

/** @def INT_ASM(number)
 *
 *  @brief Constructs the assembly wrapper function name for a fault
 *  @param number The index of the fault in the IDT
 **/
#define INT_ASM(number) _INT_ASM(number)

/** @def INT_ASM_H(number)
 *
 *  @brief Constructs a header for the assembly wrapper for a fault
 *  @param number The index of the fault in the IDT
 **/
#define INT_ASM_H(number) void _INT_ASM(number)()

/** @brief Switches to user mode
 *
 *  @param esp Stack pointer with values to be restored
 *  @return void
 **/
void go_to_user_mode(void *esp);

void set_regs();


/*****************************************************************************
 ********* FAULT INTERRUPT HANDLERS *****************************************
 *****************************************************************************/
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

/*****************************************************************************
 ********* DEVICE INTERRUPT HANDLERS *****************************************
 *****************************************************************************/
/** @brief Wrapper for the timer interrupt handler
 *  @return void
 */
NAME_ASM_H(timer_interrupt);

/** @brief Wrapper for the keyboard interrupt handler
 *  @return void
 */
NAME_ASM_H(keyboard_interrupt);

/*****************************************************************************
 ********* SYSCALL INTERRUPT HANDLERS*****************************************
 *****************************************************************************/

/** @brief Wrapper for fork syscall handler
 *  @return void
 */
NAME_ASM_H(fork_syscall);

/** @brief Wrapper for exec syscall handler
 *  @return void
 */
NAME_ASM_H(exec_syscall);

/** @brief Wrapper for set_status syscall handler
 *  @return void
 */
NAME_ASM_H(set_status_syscall);

/** @brief Wrapper for vanish syscall handler
 *  @return void
 */
NAME_ASM_H(vanish_syscall);

/** @brief Wrapper for task_vanish syscall handler
 *  @return void
 */
NAME_ASM_H(task_vanish_syscall);

/** @brief Wrapper for wait syscall handler
 *  @return void
 */
NAME_ASM_H(wait_syscall);

/** @brief Wrapper for gettid syscall handler
 *  @return void
 */
NAME_ASM_H(gettid_syscall);

/** @brief Wrapper for yield syscall handler
 *  @return void
 */
NAME_ASM_H(yield_syscall);

/** @brief Wrapper for deschedule syscall handler
 *  @return void
 */
NAME_ASM_H(deschedule_syscall);

/** @brief Wrapper for make_runnable syscall handler
 *  @return void
 */
NAME_ASM_H(make_runnable_syscall);

/** @brief Wrapper for get_ticks syscall handler
 *  @return void
 */
NAME_ASM_H(get_ticks_syscall);

/** @brief Wrapper for sleep syscall handler
 *  @return void
 */
NAME_ASM_H(sleep_syscall);

/** @brief Wrapper for thread fork syscall handler
 *  @return void
 */
NAME_ASM_H(thread_fork_syscall);

/** @brief Wrapper for new_pages syscall handler
 *  @return void
 */
NAME_ASM_H(new_pages_syscall);

/** @brief Wrapper for remove_pages syscall handler
 *  @return void
 */
NAME_ASM_H(remove_pages_syscall);

/** @brief Wrapper for getchar syscall handler
 *  @return void
 */
NAME_ASM_H(getchar_syscall);

/** @brief Wrapper for readline syscall handler
 *  @return void
 */
NAME_ASM_H(readline_syscall);

/** @brief Wrapper for print syscall handler
 *  @return void
 */
NAME_ASM_H(print_syscall);

/** @brief Wrapper for set_term_color syscall handler
 *  @return void
 */
NAME_ASM_H(set_term_color_syscall);

/** @brief Wrapper for set_cursor_pos syscall handler
 *  @return void
 */
NAME_ASM_H(set_cursor_pos_syscall);

/** @brief Wrapper for get_cursor_pos syscall handler
 *  @return void
 */
NAME_ASM_H(get_cursor_pos_syscall);

/** @brief Wrapper for halt syscall handler
 *  @return void
 */
NAME_ASM_H(halt_syscall);

/** @brief Wrapper for readfile syscall handler
 *  @return void
 */
NAME_ASM_H(readfile_syscall);

/** @brief Wrapper for misbehave syscall handler
 *  @return void
 */
NAME_ASM_H(misbehave_syscall);

/** @brief Wrapper for swexn syscall handler
 *  @return void
 */
NAME_ASM_H(swexn_syscall);

/** @brief Wrapper for udriv_register syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_register_syscall);

/** @brief Wrapper for udriv_deregister syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_deregister_syscall);

/** @brief Wrapper for udriv_send syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_send_syscall);

/** @brief Wrapper for udriv_wait syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_wait_syscall);

/** @brief Wrapper for udriv_inb syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_inb_syscall);

/** @brief Wrapper for udriv_outb syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_outb_syscall);

/** @brief Wrapper for udriv_mmap syscall handler
 *  @return void
 */
NAME_ASM_H(udriv_mmap_syscall);

#endif /* _MODE_SWITCH_H */
