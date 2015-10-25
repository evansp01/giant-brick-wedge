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
void gettid_syscall(void *addr, ureg_t state)
{
    tcb_t *p_tcb = get_tcb_from_addr(addr);
    // return the tid
    state.eax = p_tcb->id;
    return;
}
