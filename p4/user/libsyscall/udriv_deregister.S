/** @file udriv_deregister.S
 *  @brief Assembly wrapper for the udriv_deregister syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_deregister
udriv_deregister:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $UDRIV_DEREGISTER_INT # Call the udriv_deregister syscall
    popl %esi                 # Restore the value of %esi
    ret
