#include <syscall_int.h>
#include <simics.h>

.global thr_create
thr_create:
    pushl %ebp
    movl %esp, %ebp
    call alloc_frame
    movl %eax, %edx         # edx is now stack
    cmp $0, %edx
    je alloc_failed        # our frame is a null pointer
    movl 0x8(%ebp), %ecx    # get func
    movl %ecx, -0x8(%edx)   # put func on stack
    movl 0xc(%ebp), %ecx    # get arg
    movl %ecx, -0x4(%edx)   # put arg on stack
    int $THREAD_FORK_INT   # call thread fork
    movl %eax, %ecx         # store return value in ecx
    cmp $0, %ecx
    je child
    ja parent
    pushl %edx              # the failure case (pushl stack address)
    call free_frame        # free the stack
    sub $4, %esp           # restore the stack pointer
    movl %ecx, %eax         # return the value from thread_fork
    jmp exit
parent:
    movl %ecx, %eax         # return the value from thread_fork
    jmp exit
child:
    movl %edx, %esp         # set stack pointer to new stack
    movl %edx, (%esp)       # first address of the stack is the stack
    sub $8, %esp
    call thr_wrapper       # run away and never return
    nop                    # we should never get here
alloc_failed:
    movl $-1, %eax
exit:
    pop %ebp
    ret
