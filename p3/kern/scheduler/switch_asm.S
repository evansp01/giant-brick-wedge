/** @file switch.S
 *  @brief Implementation of the context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/
 
.global switch_stack_and_regs
switch_stack_and_regs:
    pusha                   # save registers
    mov 40(%esp), %eax      # get pointer to tcb
    mov %esp, %ecx          # save stack pointer
    pushl %eax              # set &tcb (arg1)
    pushl %ecx              # set saved_esp (arg0)
    call store_esp          # save esp into tcb
    add $8, %esp            # clean up stack
    mov 36(%esp), %esp      # switch esp to other kernel stack
    popa                    # restore registers     (different thread)
    ret                     # return                (different thread)

.global get_esp
get_esp:
    movl %esp, %eax
    ret
