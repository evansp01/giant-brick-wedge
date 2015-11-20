/** @file idt.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <timer_defines.h>
#include <keyhelp.h>
#include <idt.h>
#include <seg.h>
#include <simics.h>
#include <syscall_int.h>
#include <asm.h>
#include "mode_switch.h"
#include "setup_idt.h"

/** @brief Struct for Interrupt Descriptor Table (IDT) entries
 */
typedef struct {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t reserved;
    uint8_t gate_type : 3;
    uint8_t double_word : 1;
    uint8_t zero : 1;
    uint8_t privilege_level : 2;
    uint8_t present : 1;
    uint16_t offset_high;
} IDT_entry;

/** @brief Installs the handlers in the IDT
 *
 *  @return void
 */
void install_exceptions()
{
    // Set IDT entry fields and add handlers to IDT
    set_idt_exception(INT_ASM(IDT_DE), TRAP, IDT_DE);
    set_idt_exception(INT_ASM(IDT_DB), TRAP, IDT_DB);
    set_idt_exception(INT_ASM(IDT_NMI), TRAP, IDT_NMI);
    set_idt_exception(INT_ASM(IDT_BP), TRAP, IDT_BP);
    set_idt_exception(INT_ASM(IDT_OF), TRAP, IDT_OF);
    set_idt_exception(INT_ASM(IDT_BR), TRAP, IDT_BR);
    set_idt_exception(INT_ASM(IDT_UD), TRAP, IDT_UD);
    set_idt_exception(INT_ASM(IDT_NM), TRAP, IDT_NM);
    set_idt_exception(INT_ASM(IDT_DF), TRAP, IDT_DF);
    set_idt_exception(INT_ASM(IDT_CSO), TRAP, IDT_CSO);
    set_idt_exception(INT_ASM(IDT_TS), TRAP, IDT_TS);
    set_idt_exception(INT_ASM(IDT_NP), TRAP, IDT_NP);
    set_idt_exception(INT_ASM(IDT_SS), TRAP, IDT_SS);
    set_idt_exception(INT_ASM(IDT_GP), TRAP, IDT_GP);
    set_idt_exception(INT_ASM(IDT_PF), INTERRUPT, IDT_PF);
    set_idt_exception(INT_ASM(IDT_MF), TRAP, IDT_MF);
    set_idt_exception(INT_ASM(IDT_AC), TRAP, IDT_AC);
    set_idt_exception(INT_ASM(IDT_MC), TRAP, IDT_MC);
    set_idt_exception(INT_ASM(IDT_XF), TRAP, IDT_XF);
}

/** @brief Installs the system call handlers
 *
 *  @return void
 */
void install_syscalls()
{
    set_idt_syscall(NAME_ASM(fork_syscall), FORK_INT);
    set_idt_syscall(NAME_ASM(exec_syscall), EXEC_INT);
    set_idt_syscall(NAME_ASM(set_status_syscall), SET_STATUS_INT);
    set_idt_syscall(NAME_ASM(vanish_syscall), VANISH_INT);
    set_idt_syscall(NAME_ASM(task_vanish_syscall), TASK_VANISH_INT);
    set_idt_syscall(NAME_ASM(wait_syscall), WAIT_INT);

    set_idt_syscall(NAME_ASM(gettid_syscall), GETTID_INT);
    set_idt_syscall(NAME_ASM(yield_syscall), YIELD_INT);
    set_idt_syscall(NAME_ASM(deschedule_syscall), DESCHEDULE_INT);
    set_idt_syscall(NAME_ASM(make_runnable_syscall), MAKE_RUNNABLE_INT);
    set_idt_syscall(NAME_ASM(get_ticks_syscall), GET_TICKS_INT);
    set_idt_syscall(NAME_ASM(sleep_syscall), SLEEP_INT);
    set_idt_syscall(NAME_ASM(thread_fork_syscall), THREAD_FORK_INT);

    set_idt_syscall(NAME_ASM(new_pages_syscall), NEW_PAGES_INT);
    set_idt_syscall(NAME_ASM(remove_pages_syscall), REMOVE_PAGES_INT);

    set_idt_syscall(NAME_ASM(getchar_syscall), GETCHAR_INT);
    set_idt_syscall(NAME_ASM(readline_syscall), READLINE_INT);
    set_idt_syscall(NAME_ASM(print_syscall), PRINT_INT);
    set_idt_syscall(NAME_ASM(set_term_color_syscall), SET_TERM_COLOR_INT);
    set_idt_syscall(NAME_ASM(set_cursor_pos_syscall), SET_CURSOR_POS_INT);
    set_idt_syscall(NAME_ASM(get_cursor_pos_syscall), GET_CURSOR_POS_INT);

    set_idt_syscall(NAME_ASM(halt_syscall), HALT_INT);
    set_idt_syscall(NAME_ASM(readfile_syscall), READFILE_INT);
    set_idt_syscall(NAME_ASM(misbehave_syscall), MISBEHAVE_INT);
    set_idt_syscall(NAME_ASM(swexn_syscall), SWEXN_INT);
}

/** @brief Installs the device driver handlers
 *
 *  @return void
 */
void install_devices()
{
    set_idt_device(NAME_ASM(timer_interrupt), INTERRUPT, TIMER_IDT_ENTRY);
    set_idt_device(NAME_ASM(keyboard_interrupt), TRAP, KEY_IDT_ENTRY);
}

/** @brief Installs a handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param type Type of exception
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_exception(void* handler, int type, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, type, KERNEL, index);
}

/** @brief Installs a syscall handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_syscall(void* handler, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, TRAP, USER, index);
}

/** @brief Installs a device interrupt handler into the IDT
 *
 *  @param handler Pointer to the handler function
 *  @param type Type of exception
 *  @param index IDT index at which to install handler
 *  @return void
 */
void set_idt_device(void* handler, int type, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, type, KERNEL, index);
}

/** @brief Places the IDT entry in the IDT
 *
 *  @param index IDT entry index
 *  @param entry Struct to be installed in the IDT
 *  @return void
 **/
void install_idt(int index, IDT_entry* entry)
{
    *(((IDT_entry*)idt_base()) + index) = *entry;
}

/** @brief Fills in the IDT entry and adds it to the IDT
 *
 *  @param handler Address of fault handler
 *  @param segment Segment selector for destination code segment
 *  @param type The type of gate to install
 *  @param privilege Descriptor privilege level
 *  @param index IDT table index
 *  @return void
 **/
void set_idt(void* handler, int segment, int type, int privilege, int index)
{
    IDT_entry entry;
    entry.offset_low = (uint16_t)((uint32_t)handler);
    entry.segment = segment;
    entry.reserved = 0;
    entry.gate_type = type;
    entry.double_word = 1; // double word size
    entry.zero = 0;
    entry.privilege_level = privilege;
    entry.present = 1; // entry is preent
    entry.offset_high = (uint16_t)(((uint32_t)handler) >> 16);
    install_idt(index, &entry);
}
