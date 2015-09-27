.global swexn
swexn:
    pushl %esi
    leal 8(%esp), %esi
    int $0x74
    popl %esi
    ret