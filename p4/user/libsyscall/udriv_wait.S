/** @file udriv_wait.S
 *  @brief Assembly wrapper for the udriv_wait syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_wait
udriv_wait:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_WAIT_INT         # Call the udriv_wait syscall
    popl %esi                   # Restore the value of %esi
    ret