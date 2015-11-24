/** @file make_runnable.S
 *  @brief Assembly wrapper for the make_runnable syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global make_runnable
make_runnable:
    pushl %esi
    movl 8(%esp), %esi
    int $MAKE_RUNNABLE_INT
    popl %esi
    ret