/** @file misbehave.S
 *  @brief Assembly wrapper for the misbehave syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global misbehave
misbehave:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $MISBEHAVE_INT       # Call the misbehave syscall
    popl %esi                 # Restore the value of %esi
    ret
