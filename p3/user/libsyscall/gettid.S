/** @file gettid.S
 *  @brief Assembly wrapper for the gettid syscall
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#include <syscall_int.h>

.global gettid
gettid:
    int $GETTID_INT
    ret