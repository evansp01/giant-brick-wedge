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
#include <simics.h>
#include <cr.h>
#include <vm.h>

/** @brief Sets esp0 and cr3 before transitioning to user mode
 *
 *  @param addr Any address on the current kernel stack
 *  @return void
 */
void set_regs()
{
    tcb_t *tcb = get_tcb();
    pcb_t *pcb = tcb->parent;
    set_esp0((uint32_t)tcb->kernel_stack);
    switch_to(&pcb->directory);
}
