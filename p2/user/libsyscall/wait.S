/** @file wait.S
 *  @brief Assembly wrapper for the wait syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)jk
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global wait
wait:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $WAIT_INT             # Call the wait syscall
    popl %esi                 # Restore the value of %esi
    ret
