/** @file mode_switch_asm.S
 *  @brief Fault handler wrappers and mode switching code
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 */

#include <idt.h>
#include <seg.h>

/** @brief Save user mode state and switch to kernel mode */
.macro SAVE_STATE
    pusha
    pushl %gs
    pushl %fs
    pushl %es
    pushl %ds
    mov $SEGSEL_KERNEL_DS, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
.endm

/**  @brief Restores state to user mode */
.macro RESTORE_STATE
    pushl %esp                          # provide a stack address to get tcb
    call set_regs                       # set esp0 and cr3
    add $4, %esp                        # clean up function call stack
    add $8, %esp                        # clean up ureg arguments
    popl %ds
    popl %es
    popl %fs
    popl %gs
    popa
    add $4, %esp
    iret
.endm

.macro INTERRUPT_ASM_WRAPPER NAME
    .global \NAME\()_asm
    \NAME\()_asm:
        pushl $0                        # set ureg.error_code = 0
        SAVE_STATE
        pushl $0                        # set ureg.cr2 = 0
        pushl $0                        # set ureg.cause = 0
        call \NAME\()
        RESTORE_STATE
.endm

.macro EXCEPTION_ASM_WRAPPER_ERROR INT
    .global interrupt\INT\()_asm
    interrupt\INT\()_asm:
        SAVE_STATE
        pushl $0
        pushl $\INT\()
        call fault_handler
        RESTORE_STATE
.endm

.macro EXCEPTION_ASM_WRAPPER INT
    .global interrupt\INT\()_asm
    interrupt\INT\()_asm:
        pushl $0
        SAVE_STATE
        pushl $0
        pushl $\INT\()
        call fault_handler
        RESTORE_STATE
.endm

/* Assembly function for entering user mode for the first time */
.global first_entry_user_mode
first_entry_user_mode:
    movl 4(%esp), %esp      # switch stack pointers
    RESTORE_STATE           # perform the mode switch

/* Assembly wrappers for device interrupts */
INTERRUPT_ASM_WRAPPER timer_interrupt
INTERRUPT_ASM_WRAPPER keyboard_interrupt


/* Assembly wrappers for system call interrupts */
INTERRUPT_ASM_WRAPPER gettid_syscall
INTERRUPT_ASM_WRAPPER fork_syscall


/* Assembly wrappers for various system interrutps */
EXCEPTION_ASM_WRAPPER IDT_DE
EXCEPTION_ASM_WRAPPER IDT_DB
EXCEPTION_ASM_WRAPPER IDT_NMI
EXCEPTION_ASM_WRAPPER IDT_BP
EXCEPTION_ASM_WRAPPER IDT_OF
EXCEPTION_ASM_WRAPPER IDT_BR
EXCEPTION_ASM_WRAPPER IDT_UD
EXCEPTION_ASM_WRAPPER IDT_NM
EXCEPTION_ASM_WRAPPER_ERROR IDT_DF
EXCEPTION_ASM_WRAPPER IDT_CSO
EXCEPTION_ASM_WRAPPER_ERROR IDT_TS
EXCEPTION_ASM_WRAPPER_ERROR IDT_NP
EXCEPTION_ASM_WRAPPER_ERROR IDT_SS
EXCEPTION_ASM_WRAPPER_ERROR IDT_GP
EXCEPTION_ASM_WRAPPER_ERROR IDT_PF
EXCEPTION_ASM_WRAPPER IDT_MF
EXCEPTION_ASM_WRAPPER_ERROR IDT_AC
EXCEPTION_ASM_WRAPPER IDT_MC
EXCEPTION_ASM_WRAPPER IDT_XF
    
