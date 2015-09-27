.global make_runnable
make_runnable:
    pushl %esi
    movl 8(%esp), %esi
    int $0x47
    popl %esi
    ret