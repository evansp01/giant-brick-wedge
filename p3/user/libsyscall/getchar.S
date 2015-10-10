/** @file getchar.S
 *  @brief Assembly wrapper for the getchar syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global getchar
getchar:
    int $GETCHAR_INT
    ret