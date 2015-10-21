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
#include <mode_switch.h>
#include <idt.h>
#include <seg.h>
#include <simics.h>
#include <setup_idt.h>
#include <syscall_int.h>
#include <asm.h>

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
    set_idt_exception(INT_ASM(IDT_PF), TRAP, IDT_PF);
    set_idt_exception(INT_ASM(IDT_MF), TRAP, IDT_MF);
    set_idt_exception(INT_ASM(IDT_AC), TRAP, IDT_AC);
    set_idt_exception(INT_ASM(IDT_MC), TRAP, IDT_MC);
    set_idt_exception(INT_ASM(IDT_XF), TRAP, IDT_XF);
}

void install_syscalls()
{
    set_idt_syscall(NAME_ASM(gettid_syscall), GETTID_INT);
}

void install_devices()
{
    set_idt_device(NAME_ASM(timer_interrupt), TRAP, TIMER_IDT_ENTRY);
    set_idt_device(NAME_ASM(keyboard_interrupt), TRAP, KEY_IDT_ENTRY);
}

void set_idt_exception(void* handler, int type, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, type, KERNEL, index);
}

void set_idt_syscall(void* handler, int index)
{
    set_idt(handler, SEGSEL_KERNEL_CS, TRAP, USER, index);
}

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