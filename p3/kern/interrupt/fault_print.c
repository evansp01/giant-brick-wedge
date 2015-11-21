/** @file fault_print.c
 *  @brief Implementation of functions to print debug messages for faults
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <idt.h>
#include <simics.h>
#include <ureg.h>
#include <stdint.h>
#include <common_kern.h>
#include <eflags.h>
#include <vm.h>
#include <debug_print.h>

/** @brief Gets a specified bit from a word */
#define GET_BIT(bit, value) (((value) >> (bit)) & 1)
/** @brief Prints the contents of two segmentation registers */
#define DUMP_SEGS(name1, value1, name2, value2) \
    KPRINTF("%s = 0x%04x, %s = 0x%04x", name1,  \
            (uint16_t)value1, name2, (uint16_t)value2)
/** @brief Prints the contents of the 32 and 16 bit components of a register */
#define DUMP_REG(name, short, value)           \
    KPRINTF("%s = 0x%08lx, %s = 0x%04x", name, \
            (uint32_t)value, short, (uint16_t)value)
/** @brief Prints the contents a segment and a register */
#define DUMP_SEGR(name1, value1, name2, value2) \
    KPRINTF("%s = 0x%04x, %s = 0x%08lx", name1, \
            (uint16_t)value1, name2, (uint32_t)value2)

/** @brief Identifies exceptions based on IDT indexes
 *
 *  @param code IDT index of exception
 *  @return String containing type of exception
 **/
char* exception_name(int code)
{
    switch (code) {
    case IDT_DE:
        return "Division Error";
    case IDT_DB:
        return "Debug Exception";
    case IDT_NMI:
        return "Non-Maskable Interrupt";
    case IDT_BP:
        return "Breakpoint";
    case IDT_OF:
        return "Overflow";
    case IDT_BR:
        return "Bound Range Exceeded";
    case IDT_UD:
        return "Undefined Opcode";
    case IDT_NM:
        return "No Math Coprocessor";
    case IDT_DF:
        return "Double Fault";
    case IDT_CSO:
        return "Coprocessor Segment Overrun";
    case IDT_TS:
        return "Invalid Task Segment Selector";
    case IDT_NP:
        return "Segment Not Present";
    case IDT_SS:
        return "Stack Segment Fault";
    case IDT_GP:
        return "General Protection Fault";
    case IDT_PF:
        return "Page Fault";
    case IDT_MF:
        return "X87 Math Fault";
    case IDT_AC:
        return "Alignment Check";
    case IDT_MC:
        return "Machine Check";
    case IDT_XF:
        return "SSE Floating Point Exception";
    default:
        return "Unknown";
    }
}

/** @brief Prints out the given register state
 *
 *  @param ureg Struct containing saved register values
 *  @return void
 **/
void dump_registers(ureg_t* ureg)
{
    KPRINTF("%s (Exception %d)", exception_name(ureg->cause), ureg->cause);
    KPRINTF("Error code: %d", ureg->error_code);
    DUMP_SEGR("cs", ureg->cs, "eip", ureg->eip);
    if (ureg->eip > USER_MEM_START) {
        DUMP_SEGR("ss", ureg->ss, "esp", ureg->esp);
    }
    DUMP_SEGS("ds", ureg->ds, "es", ureg->es);
    DUMP_SEGS("fs", ureg->fs, "gs", ureg->gs);
    DUMP_REG("eax", "ax", ureg->eax);
    DUMP_REG("ecx", "cx", ureg->ecx);
    DUMP_REG("edx", "dx", ureg->edx);
    DUMP_REG("ebx", "bx", ureg->ebx);
    DUMP_REG("ebp", "bp", ureg->ebp);
    DUMP_REG("esi", "si", ureg->esi);
    DUMP_REG("edi", "di", ureg->edi);
    KPRINTF(
        "eflags = %d %d %d %d %d %d %d %d %d %d %d"
        " %d %d %d %d %d %d %d %d %d %d %d = 0x%x",
        GET_BIT(21, ureg->eflags),
        GET_BIT(20, ureg->eflags),
        GET_BIT(19, ureg->eflags),
        GET_BIT(18, ureg->eflags),
        GET_BIT(17, ureg->eflags),
        GET_BIT(16, ureg->eflags),
        GET_BIT(15, ureg->eflags),
        GET_BIT(14, ureg->eflags),
        GET_BIT(13, ureg->eflags),
        GET_BIT(12, ureg->eflags),
        GET_BIT(11, ureg->eflags),
        GET_BIT(10, ureg->eflags),
        GET_BIT(9, ureg->eflags),
        GET_BIT(8, ureg->eflags),
        GET_BIT(7, ureg->eflags),
        GET_BIT(6, ureg->eflags),
        GET_BIT(5, ureg->eflags),
        GET_BIT(4, ureg->eflags),
        GET_BIT(3, ureg->eflags),
        GET_BIT(2, ureg->eflags),
        GET_BIT(1, ureg->eflags),
        GET_BIT(0, ureg->eflags),
        ureg->eflags);
    KPRINTF("         I V V A V R - N I I O D I T S Z - A - P - C");
    KPRINTF("         D I I C M F   T O O F F F F F F   F   F   F");
    KPRINTF("                         P F");
    KPRINTF("                         L L");
}


