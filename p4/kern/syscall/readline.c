/** @file readline.c
 *
 *  @brief Keyboard interrupt handler and code to handle readline requests
 *
 *  @author Evan Palmer (esp)
 *  @author Jonathan Ong (jonathao)
 *  @bug No known bugs
 **/
#include <ureg.h>

/** @brief The readline syscall
 *  @param state The current state in user mode
 *  @return void
 */
void readline_syscall(ureg_t state)
{
    state.eax = -1;
}
