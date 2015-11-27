/** @file udriv_inb.S
 *  @brief Assembly wrapper for the udriv_inb syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_inb
udriv_inb:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_INB_INT          # Call the udriv_inb syscall
    popl %esi                   # Restore the value of %esi
    ret