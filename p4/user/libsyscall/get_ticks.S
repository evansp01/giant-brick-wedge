/** @file get_ticks.S
 *  @brief Assembly wrapper for the get_ticks syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global get_ticks
get_ticks:
    int $GET_TICKS_INT
    ret