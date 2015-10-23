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

/** @brief Generic fault handler
 *
 *  @param state Struct containing saved register state before exception
 *  @return void
 **/
void fault_handler(tcb_t *ptr, ureg_t state)
{
    dump_registers(&state);
}
