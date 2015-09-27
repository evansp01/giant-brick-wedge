.global readline
readline:
    pushl %esi
    leal 8(%esp), %esi
    int $0x4D
    popl %esi
    ret