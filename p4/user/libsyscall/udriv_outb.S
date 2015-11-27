/** @file udriv_outb.S
 *  @brief Assembly wrapper for the udriv_outb syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_outb
udriv_outb:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_OUTB_INT         # Call the udriv_outb syscall
    popl %esi                   # Restore the value of %esi
    ret