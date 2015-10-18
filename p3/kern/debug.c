#include <stdio.h>
#include <simics.h> /* lprintf() */
#include <eflags.h>


#define EFL_FORMAT(name, value) lprintf("%32s: %d", name, value)
#define EFL_PRINT(eflags, mask, name) EFL_FORMAT(name, !!((eflags) & (mask)))

void print_eflags()
{
    uint32_t eflags = get_eflags();
    int iopl = (eflags >> EFL_IOPL_SHIFT) & 3;
    EFL_PRINT(eflags, EFL_CF, "carry flag");
    EFL_PRINT(eflags, EFL_RESV1, "reserved1 (1)");
    EFL_PRINT(eflags, EFL_PF, "parity flag");
    EFL_PRINT(eflags, EFL_RESV2, "reserved2 (0)");
    EFL_PRINT(eflags, EFL_AF, "auxiliary");
    EFL_PRINT(eflags, EFL_RESV3, "reserved3 (0)");
    EFL_PRINT(eflags, EFL_ZF, "zero flag");
    EFL_PRINT(eflags, EFL_SF, "sign flag");
    EFL_PRINT(eflags, EFL_TF, "trap flag");
    EFL_PRINT(eflags, EFL_IF, "interrupt flag");
    EFL_PRINT(eflags, EFL_DF, "direction flag");
    EFL_PRINT(eflags, EFL_OF, "overflow flag");
    EFL_FORMAT("io privaledge", iopl);
    EFL_PRINT(eflags, EFL_NT, "nested task");
    EFL_PRINT(eflags, EFL_RESV4, "reserved4 (0)");
    EFL_PRINT(eflags, EFL_RF, "resume flag");
    EFL_PRINT(eflags, EFL_VM, "virtual 8086");
    EFL_PRINT(eflags, EFL_AC, "alignment check");
    EFL_PRINT(eflags, EFL_VIF, "virtual interrupt flag");
    EFL_PRINT(eflags, EFL_VIP, "virtual interrupt pending");
    EFL_PRINT(eflags, EFL_ID, "identification support");
}


