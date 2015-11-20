/** @file fault.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <idt.h>
#include <simics.h>
#include <ureg.h>
#include <stdint.h>
#include <control.h>
#include <stdlib.h>
#include <cr.h>
#include <setup_idt.h>
#include <stack_info.h>
#include <mode_switch.h>
#include <common_kern.h>
#include <syscall_kern.h>
#include <seg.h>
#include <asm.h>

void dump_registers(ureg_t* ureg);

void default_fault_handler(ureg_t* state, tcb_t* tcb)
{
    if (tcb->swexn.handler != NULL) {
        swexn_handler(state, tcb);
    }
    // Print error message
    dump_registers(state);
    // Kill thread on error
    vanish_thread(tcb, THREAD_EXIT_FAILED);
}

void page_fault_handler(ureg_t* state, tcb_t* tcb)
{
    int vm_status;
    state->cr2 = get_cr2();
    // page fault is an interrupt gate, so we must reenable interrupts
    // after we get the fault
    enable_interrupts();
    ppd_t* ppd = tcb->process->directory;
    mutex_lock(&ppd->lock);
    vm_status = vm_resolve_pagefault(ppd, state->cr2, state->error_code);
    mutex_unlock(&ppd->lock);
    if (vm_status < 0) {
        default_fault_handler(state, tcb);
    }
}



/** @brief Generic fault handler
 *
 *  @param state Struct containing saved register state before exception
 *  @return void
 **/
void fault_handler(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    if(state.cs == SEGSEL_KERNEL_CS){
        panic("Thread crashed in kernel space");
    }

    switch (state.cause) {
    //page faults
    case IDT_PF:
        page_fault_handler(&state, tcb);
        break;
    // Traps
    case IDT_DB:
    case IDT_BP:
    case IDT_OF:
    // Faults
    case IDT_DE:
    case IDT_NMI:
    case IDT_BR:
    case IDT_UD:
    case IDT_NM:
    case IDT_DF:
    case IDT_CSO:
    case IDT_TS:
    case IDT_NP:
    case IDT_SS:
    case IDT_GP:
    case IDT_MF:
    case IDT_AC:
    case IDT_MC:
    case IDT_XF:
        default_fault_handler(&state, tcb);
        break;
    default:
        lprintf("Never heard of cause %d, you sure about that?", state.cause);
        panic("Unknown fault");
        break;
    }
}
