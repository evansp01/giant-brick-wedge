/** @file readline.S
 *  @brief Assembly wrapper for the readline syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global sys_readline
sys_readline:
    pushl %esi
    leal 8(%esp), %esi
    int $READLINE_INT
    popl %esi
    ret