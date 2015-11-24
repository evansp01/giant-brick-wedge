/** @file set_cursor_pos.S
 *  @brief Assembly wrapper for the set_cursor_pos syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global set_cursor_pos
set_cursor_pos:
    pushl %esi
    leal 8(%esp), %esi
    int $SET_CURSOR_POS_INT
    popl %esi
    ret