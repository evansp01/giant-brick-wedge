/** @file print.S
 *  @brief Assembly wrapper for the print syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global sys_print
sys_print:
    pushl %esi                # Save old %esi value
    leal 8(%esp), %esi        # Get the first argument
    int $PRINT_INT            # Call the print syscall
    popl %esi                 # Restore the value of %esi
    ret
