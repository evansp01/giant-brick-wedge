/** @file swexn.S
 *  @brief Assembly wrapper for the swexn syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global swexn
swexn:
    pushl %esi
    leal 8(%esp), %esi
    int $SWEXN_INT
    popl %esi
    ret