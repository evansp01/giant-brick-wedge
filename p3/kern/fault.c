/** @file fault.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <fault.h>
#include <asm.h>
#include <idt.h>
#include <seg.h>
#include <simics.h>


#define GET_LOW(addr)  ((unsigned int)addr & 0xFFFF)
#define GET_HIGH(addr) ((unsigned int)addr >> 16)
#define TRAP 0x7
#define INTERRUPT 0x6
#define KERNEL 0
#define USER 3
/** @brief Struct for Interrupt Descriptor Table (IDT) entries
 */
typedef struct {
  uint16_t offset_low;
  uint16_t segment;
  uint8_t reserved;
  uint8_t gate_type : 3;
  uint8_t D : 1;
  uint8_t zero : 1;
  uint8_t DPL : 2;
  uint8_t P : 1;
  uint16_t offset_high;
} IDT_entry;


/** @brief Installs the handlers in the IDT
 *
 *  @return Void
 */
int handler_install()
{
  // Set IDT entry fields and add handlers to IDT
  set_idt(divide_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DE);
  set_idt(debug_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DB);
  set_idt(nmi_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NMI);
  set_idt(breakpoint_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_BP);
  set_idt(overflow_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_OF);
  set_idt(bound_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_BR);
  set_idt(opcode_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_UD);
  set_idt(no_math_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NM);
  set_idt(double_fault_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DF);
  set_idt(cso_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_CSO);
  set_idt(tss_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_TS);
  set_idt(not_present_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NP);
  set_idt(stack_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_SS);
  set_idt(protection_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_GP);
  set_idt(page_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_PF);
  set_idt(math_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_MF);
  set_idt(alignment_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_AC);
  set_idt(machine_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_MC);
  set_idt(fpu_handler_asm, SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_XF);
  return 0;
}

/** @brief Places the IDT entry in the IDT
 *
 *  @param index IDT entry index
 *  @param entry Struct to be installed in the IDT
 *  @return Void
 **/
void install_idt(int index, IDT_entry* entry)
{
    *(((IDT_entry*)idt_base()) + index) = *entry;
}

/** @brief Fills in the IDT entry and installs it
 *
 *  @param handler Address of fault handler
 *  @param segment Segment selector for destination code segment
 *  @param type The type of gate to install
 *  @param privilege Descriptor privilege level
 *  @param index IDT table index
 *  @return Void
 **/
void set_idt(void *handler, int segment, int type, int privilege, int index)
{
    IDT_entry entry;
    entry.offset_low = GET_LOW(handler);
    entry.segment = segment;
    entry.reserved = 0;
    entry.gate_type = type;
    entry.D = 1;                     // double word size
    entry.zero = 0;
    entry.DPL = privilege;
    entry.P = 1;                     // entry is preent
    entry.offset_high = GET_HIGH(handler);
    install_idt(index, &entry);
}

/** @brief Handler function for divide error exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(divide)
{
    lprintf("Fault: Divide handler triggered!");
    return;
}

/** @brief Handler function for debug exceptions (FAULT/TRAP)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(debug)
{
    lprintf("Fault: Debug handler triggered!");
    return;
}

/** @brief Handler function for non-maskable interrupts (INTERRUPT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(nmi)
{
    lprintf("Fault: NMI handler triggered!");
    return;
}

/** @brief Handler function for breakpoint exceptions (TRAP)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(breakpoint)
{
    lprintf("Fault: Breakpoint handler triggered!");
    return;
}

/** @brief Handler function for overflow exceptions (TRAP)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(overflow)
{
    lprintf("Fault: Overflow handler triggered!");
    return;
}

/** @brief Handler function for BOUND range exceeded exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(bound)
{
    lprintf("Fault: BOUND handler triggered!");
    return;
}

/** @brief Handler function for invalid opcode exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(opcode)
{
    lprintf("Fault: Invalid opcode handler triggered!");
    return;
}

/** @brief Handler function for no math coprocessor exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(no_math)
{
    lprintf("Fault: No math coprocessor handler triggered!");
    return;
}

/** @brief Handler function for double fault exceptions (ABORT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(double_fault)
{
    lprintf("Fault: Double fault handler triggered!");
    return;
}

/** @brief Handler function for coprocessor segment overrun exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(cso)
{
    lprintf("Fault: Coprocessor segment overrun handler triggered!");
    return;
}

/** @brief Handler function for invalid task segment selector exceptions (ABORT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(tss)
{
    lprintf("Fault: Invalid task segment selector handler triggered!");
    return;
}

/** @brief Handler function for segment not present exceptions (ABORT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(not_present)
{
    lprintf("Fault: Segment not present handler triggered!");
    return;
}

/** @brief Handler function for stack segment exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(stack)
{
    lprintf("Fault: Stack segment exception handler triggered!");
    return;
}

/** @brief Handler function for general protection exceptions (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(protection)
{
    lprintf("Fault: General protection error handler triggered!");
    return;
}

/** @brief Handler function for page faults (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(page)
{
    MAGIC_BREAK;
    lprintf("Fault: Page fault handler triggered!");
    while(1) {
        continue;
    }
    // if (kernel) panic();
    // else if (write_to_zero_page) get_reserved_page();
    // else if (page_table_missing) segfault();
    return;
}

/** @brief Handler function for x87 FPU floating point error (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(math)
{
    lprintf("Fault: x87 FPU floating point fault handler triggered!");
    return;
}

/** @brief Handler function for alignment check faults (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(alignment)
{
    lprintf("Fault: Alignment check fault handler triggered!");
    return;
}

/** @brief Handler function for machine check faults (ABORT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(machine)
{
    lprintf("Fault: Machine check fault handler triggered!");
    return;
}

/** @brief Handler function for SIMD floating point faults (FAULT)
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(fpu)
{
    lprintf("Fault: SIMD floating point fault handler triggered!");
    return;
}



