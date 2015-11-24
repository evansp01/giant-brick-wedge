/** @file sleep.S
 *  @brief Assembly wrapper for the sleep syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global sleep
sleep:
    pushl %esi
    movl 8(%esp), %esi
    int $SLEEP_INT
    popl %esi
    ret