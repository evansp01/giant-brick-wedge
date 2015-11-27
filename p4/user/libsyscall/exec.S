/** @file exec.S
 *  @brief Assembly wrapper for the exec syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global sys_exec
sys_exec:
    pushl %esi                # Save old %esi value
    leal 8(%esp), %esi        # Get the first argument
    int $EXEC_INT             # Call the exec syscall
    popl %esi                 # Restore the value of %esi
    ret
