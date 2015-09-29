#include <syscall_int.h>

.global thr_create
thr_create:
    push %ebp
    mov %esp, %ebp
    call alloc_frame
    mov %eax, %edx         # edx is now stack
    mov 0x8(%ebp), %ecx    # get func
    mov %ecx, -0x8(%edx)   # put func on stack
    mov 0xc(%ebp), %ecx    # get arg
    mov %ecx, -0x4(%edx)   # put arg on stack
    int $THREAD_FORK_INT   # call thread fork
    mov %eax, %ecx         # store return value in ecx
    cmp %ecx, 0
    je child
    ja parent
    push %edx              # the failure case (push stack address)
    call free_frame        # free the stack
    sub $4, %esp           # restore the stack pointer
    mov %ecx, %eax         # return the value from thread_fork
    jmp exit
parent:
    mov %ecx, %eax         # return the value from thread_fork
    jmp exit
child:
    mov %edx, %esp         # set stack pointer to new stack
    mov %edx, (%esp)       # first address of the stack is the stack
    sub $8, %esp
    call thr_wrapper       # run away and never return
    nop                    # we should never get here
exit:
    pop %ebp
    ret
