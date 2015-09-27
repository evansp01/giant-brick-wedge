.global deschedule
deschedule:
    pushl %esi
    movl 8(%esp), %esi
    int $0x46
    popl %esi
    ret