/** @file udriv_mmap.S
 *  @brief Assembly wrapper for the udriv_mmap syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global udriv_mmap
udriv_mmap:
    pushl %esi                  # Save old %esi value
    leal 8(%esp), %esi          # Get the pointer to arguments
    int $UDRIV_MMAP_INT         # Call the udriv_mmap syscall
    popl %esi                   # Restore the value of %esi
    ret