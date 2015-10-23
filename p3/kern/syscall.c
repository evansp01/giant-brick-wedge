/** @file syscall.c
 *
 *  @brief Functions to perform fault handling
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/

#include <simics.h>
#include <ureg.h>
#include <stdint.h>
#include <common_kern.h>
#include <control.h>


/** @brief Handler function for gettid()
 *
 *  @return void
 */
void gettid_syscall(tcb_t *p_tcb, ureg_t state)
{
    // return the tid
    state.eax = p_tcb->id;
    return;
}
