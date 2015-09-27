/** @file deschedule.S
 *  @brief Assembly wrapper for the deschedule syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global deschedule
deschedule:
    pushl %esi
    movl 8(%esp), %esi
    int $DESCHEDULE_INT
    popl %esi
    ret