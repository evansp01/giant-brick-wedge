/** @file atomic.S
 *  @brief Some atomic operations in assembly
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

.global atomic_xchg
atomic_xchg:
        movl    4(%esp),  %ecx      # get *ptr
        movl    8(%esp),  %eax      # get value
  lock  xchg    (%ecx),   %eax      # perform xchg
        ret

.global atomic_cas
atomic_cas:
        movl    4(%esp),  %ecx      # get *ptr
        movl    8(%esp),  %edx      # get newval
        movl    12(%esp), %eax      # get oldval
  lock  cmpxchg %edx,     (%ecx)    # perform cas
        ret

.global atomic_inc
atomic_inc:
        movl 4(%esp), %ecx
  lock  incl (%ecx)                 # increment
        ret

.global atomic_dec
atomic_dec:
        movl 4(%esp), %ecx
  lock  decl (%ecx)                 # decrement
        ret
