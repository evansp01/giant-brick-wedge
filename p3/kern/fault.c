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
#include <syscall_int.h>
#include <ureg.h>

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
 *  @return Void
 */
int handler_install()
{
    // Set IDT entry fields and add handlers to IDT
    set_idt(INT_ASM(IDT_DE), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DE);
    set_idt(INT_ASM(IDT_DB), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DB);
    set_idt(INT_ASM(IDT_NMI), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NMI);
    set_idt(INT_ASM(IDT_BP), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_BP);
    set_idt(INT_ASM(IDT_OF), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_OF);
    set_idt(INT_ASM(IDT_BR), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_BR);
    set_idt(INT_ASM(IDT_UD), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_UD);
    set_idt(INT_ASM(IDT_NM), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NM);
    set_idt(INT_ASM(IDT_DF), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_DF);
    set_idt(INT_ASM(IDT_CSO), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_CSO);
    set_idt(INT_ASM(IDT_TS), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_TS);
    set_idt(INT_ASM(IDT_NP), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_NP);
    set_idt(INT_ASM(IDT_SS), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_SS);
    set_idt(INT_ASM(IDT_GP), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_GP);
    set_idt(INT_ASM(IDT_PF), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_PF);
    set_idt(INT_ASM(IDT_MF), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_MF);
    set_idt(INT_ASM(IDT_AC), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_AC);
    set_idt(INT_ASM(IDT_MC), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_MC);
    set_idt(INT_ASM(IDT_XF), SEGSEL_KERNEL_CS, TRAP, KERNEL, IDT_XF);

    set_idt(gettid_handler_asm, SEGSEL_KERNEL_CS, TRAP, USER, GETTID_INT);

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

#define GET_BIT(bit, value) (((value) >> (bit)) & 1)
#define DUMP_SEGS(name1, value1, name2, value2) \
    lprintf("%s = 0x%04x, %s = 0x%04x", name1,  \
            (uint16_t)value1, name2, (uint16_t)value2)
#define DUMP_REG(name, short, value)           \
    lprintf("%s = 0x%08lx, %s = 0x%04x", name, \
            (uint32_t)value, short, (uint16_t)value)
#define DUMP_SEGR(name1, value1, name2, value2) \
    lprintf("%s = 0x%04x, %s = 0x%08lx", name1, \
            (uint16_t)value1, name2, (uint32_t)value2)

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

void dump_registers(ureg_t* ureg)
{
    lprintf("%s (Exception %d)", exception_name(ureg->cause), ureg->cause);
    lprintf("Error code: %d", ureg->error_code);
    DUMP_SEGR("cs", ureg->cs, "eip", ureg->eip);
    DUMP_SEGR("ss", ureg->ss, "esp", ureg->esp);
    DUMP_SEGS("ds", ureg->ds, "es", ureg->es);
    DUMP_SEGS("fs", ureg->fs, "gs", ureg->gs);
    DUMP_REG("eax", "ax", ureg->eax);
    DUMP_REG("ecx", "cx", ureg->ecx);
    DUMP_REG("edx", "dx", ureg->edx);
    DUMP_REG("ebx", "bx", ureg->ebx);
    DUMP_REG("ebp", "bp", ureg->ebp);
    DUMP_REG("esi", "si", ureg->esi);
    DUMP_REG("edi", "di", ureg->edi);
    lprintf(
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
    lprintf("         I V V A V R - N I I O D I T S Z - A - P - C");
    lprintf("         D I I C M F   T O O F F F F F F   F   F   F");
    lprintf("                         P F");
    lprintf("                         L L");
}

void fault_handler(ureg_t state)
{
    dump_registers(&state);
}

/** @brief Handler function for gettid()
 *
 *  @return Void
 */
CONSTRUCT_HANDLER_C(gettid)
{
    lprintf("Running gettid() handler");

    //MAGIC_BREAK;

    return;
}
