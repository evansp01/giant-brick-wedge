/** @file udriv_send.S
 *  @brief Assembly wrapper for the udriv_send syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_send
udriv_send:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_SEND_INT         # Call the udriv_send syscall
    popl %esi                   # Restore the value of %esi
    ret