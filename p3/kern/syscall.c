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


/** @brief Handler function for gettid()
 *
 *  @return void
 */
void gettid_syscall(ureg_t state)
{
    lprintf("Running gettid() handler");

    state.eax = 1;

    return;
}
