/** @file set_term_color.S
 *  @brief Assembly wrapper for the set_term_color syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global set_term_color
set_term_color:
    pushl %esi
    movl 8(%esp), %esi
    int $SET_TERM_COLOR_INT
    popl %esi
    ret