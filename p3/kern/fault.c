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

/** @brief Generic fault handler
 *
 *  @param state Struct containing saved register state before exception
 *  @return void
 **/
void fault_handler(tcb_t *ptr, ureg_t state)
{
    // Print error message
    dump_registers(&state);
    
    // Handle fault
    switch (state.cause) {
        
        // Handle page faults
        case IDT_PF:
            // TODO: Do something here eventually
            panic("Page fault!");
            return;
        
        // Potentially solvable?
        case IDT_DE:
        case IDT_DB:
        case IDT_BP:
        case IDT_OF:
            // TODO: Do something here, maybe
            panic("Potentially solvable faults");
            return;
        
        // *Abort*
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
        default:
            // TODO: Kill the process
            panic("Process is terminally ill");
            return;
    }
}
