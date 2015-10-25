/** @file mode_switch.c
 *
 *  @brief Functions for mode switching
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <mode_switch.h>
#include <control.h>
#include <stdlib.h>
#include <cr.h>
#include <simics.h>

/** @brief Sets esp0 to the top of current kernel stack
 *
 *  @param addr Any address on the current kernel stack
 *  @return void
 */
void set_esp0_wrapper(void *addr)
{
    tcb_t *tcb = get_tcb_from_addr(addr);
    set_esp0((uint32_t)tcb->kernel_stack);
}
