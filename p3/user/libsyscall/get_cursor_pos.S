/** @file get_cursor_pos.S
 *  @brief Assembly wrapper for the get_cursor_pos syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global get_cursor_pos
get_cursor_pos:
    pushl %esi
    leal 8(%esp), %esi
    int $GET_CURSOR_POS_INT
    popl %esi
    ret