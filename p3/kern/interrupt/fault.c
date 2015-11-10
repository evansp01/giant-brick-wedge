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
#include <utilities.h>
#include <control.h>
#include <stdlib.h>
#include <cr.h>
#include <setup_idt.h>
#include <stack_info.h>
#include <loader.h>
#include <mode_switch.h>

void default_fault_handler(ureg_t* state, tcb_t* tcb)
{
    // Print error message
    dump_registers(state);
    // and panic, should probably kill thread
    panic("Process is terminally ill");
}

void page_fault_handler(ureg_t* state, tcb_t* tcb)
{
    int vm_status;
    state->cr2 = get_cr2();
    ppd_t* ppd = &tcb->process->directory;
    mutex_lock(&ppd->lock);
    vm_status = vm_resolve_pagefault(ppd, state->cr2, state->error_code);
    mutex_unlock(&ppd->lock);
    if (vm_status < 0) {
        default_fault_handler(state, tcb);
    }
}

/** @brief Swexn handler
 *
 *  @param state Struct containing saved register state before exception
 *  @param tcb TCB of current thread
 *  @return void
 **/
void swexn_handler(ureg_t* state, tcb_t* tcb)
{
    // Copy then deregister current software exception handler
    extern swexn_t swexn;
    swexn_t s = swexn;
    deregister_swexn();
    
    // Setup exception stack
    swexn_stack_t swexn_stack;
    swexn_stack.ret_addr = 0;
    swexn_stack.arg = s.arg;
    swexn_stack.ureg = (void *)((uint32_t)s.stack - sizeof(ureg_t));
    swexn_stack.state = *state;
    void *start = (void *)((uint32_t)s.stack - sizeof(swexn_stack_t));
    vm_write(&tcb->process->directory, &swexn_stack, start, 
        sizeof(swexn_stack_t));
    
    // Setup context to switch to exception handler
    void *new_esp = create_context((uint32_t)tcb->kernel_stack,
        (uint32_t)start, (uint32_t)s.handler);
        
    // Run software exception handler
    go_to_user_mode(new_esp);
    
    // Should never reach here
    panic("We are lost in the depths of swexn");
}

/** @brief Generic fault handler
 *
 *  @param state Struct containing saved register state before exception
 *  @return void
 **/
void fault_handler(ureg_t state)
{
    tcb_t* tcb = get_tcb();
    
    extern swexn_t swexn;
    if (swexn.handler != NULL) {
        swexn_handler(&state, tcb);
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
