/** @file remove_pages.S
 *  @brief Assembly wrapper for the remove_pages syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global remove_pages
remove_pages:
    pushl %esi                # Save old %esi value
    movl 8(%esp), %esi        # Get the first argument
    int $REMOVE_PAGES_INT     # Call the remove_pages syscall
    popl %esi                 # Restore the value of %esi
    ret
