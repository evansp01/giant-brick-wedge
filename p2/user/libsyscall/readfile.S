/** @file readfile.S
 *  @brief Assembly wrapper for the readfile syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global readfile
readfile:
    pushl %esi                # Save old %esi value
    leal 8(%esp), %esi        # Get the first argument
    int $READFILE_INT         # Call the readfile syscall
    popl %esi                 # Restore the value of %esi
    ret
