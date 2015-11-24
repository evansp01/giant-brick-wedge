/** @file set_status.S
 *  @brief Assembly wrapper for the set_status syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global set_status
set_status:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $SET_STATUS_INT       # Call the set_status syscall
    popl %esi                 # Restore the value of %esi
    ret
