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


void fault_handler(ureg_t state)
{
    dump_registers(&state);
}
