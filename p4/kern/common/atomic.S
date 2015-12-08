/** @file atomic.S
 *  @brief Some atomic operations in assembly
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

.global atomic_xadd
atomic_xadd:
        movl 4(%esp), %ecx
        movl 8(%esp), %edx
  lock  xadd %edx, (%ecx)           # increment and return original value
        movl %edx, %eax
        ret