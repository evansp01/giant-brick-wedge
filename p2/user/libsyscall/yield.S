/** @file yield.S
 *  @brief Assembly wrapper for the yield syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global yield
yield:
    pushl %esi
    movl 8(%esp), %esi
    int $YIELD_INT
    popl %esi
    ret