/** @file vanish.S
 *  @brief Assembly wrapper for the vanish syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)jk
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global vanish
vanish:
    int $VANISH_INT
