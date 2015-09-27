/** @file fork.S
 *  @brief Assembly wrapper for the fork syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global fork
fork:
    int $FORK_INT
