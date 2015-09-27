.global yield
yield:
    pushl %esi
    movl 8(%esp), %esi
    int $0x45
    popl %esi
    ret