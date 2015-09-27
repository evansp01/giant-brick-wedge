/** @file task_vanish.S
 *  @brief Assembly wrapper for the task_vanish syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global task_vanish
task_vanish:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $TASK_VANISH_INT      # Call the task_vanish syscall
    popl %esi                 # Restore the value of %esi
    ret
