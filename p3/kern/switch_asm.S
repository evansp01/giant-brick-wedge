/** @file switch.S
 *  @brief Implementation of the context and mode switching functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/
 
.global switch_context
switch_context:
    pusha
    mov 40(%esp), %eax      # get pointer to tcb
    mov %esp, 28(%eax)      # store saved esp into tcb
    mov 36(%esp), %esp      # switch esp to other kernel stack
    popa                    # restore general purpose registers
    ret

    
/*
.global yield
yield:
    mov 4(%esp), %edx
    push %edx
    call get_tcb_to_schedule
    mov %eax, %ecx
    mov %ecx, (%esp)
    call prepare_to_schedule
    sub $4, %esp
    # eax is the stack of the other dude
    pusha
    mov %eax, %esp
    mov %ecx, %eax
    popa
    push %eax
    call restore_state
    sub $4, %esp
    ret
*/