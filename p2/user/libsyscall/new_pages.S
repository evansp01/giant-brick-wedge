/** @file new_pages.S
 *  @brief Assembly wrapper for the new_pages syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global new_pages
new_pages:
    pushl %esi                # Save old %esi value
    leal 8(%esp), %esi        # Get the first argument
    int $NEW_PAGES_INT        # Call the new_pages syscall
    popl %esi                 # Restore the value of %esi
    ret
