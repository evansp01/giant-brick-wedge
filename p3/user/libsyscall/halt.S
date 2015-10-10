/** @file halt.S
 *  @brief Assembly wrapper for the halt syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global halt
halt:
    int $HALT_INT
