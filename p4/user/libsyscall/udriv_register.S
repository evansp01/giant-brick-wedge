/** @file udriv_register.S
 *  @brief Assembly wrapper for the udriv_register syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_register
udriv_register:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_REGISTER_INT     # Call the udriv_register syscall
    popl %esi                   # Restore the value of %esi
    ret