.global print
print:
    pushl %esi
    leal 8(%esp), %esi
    int $0x4E
    popl %esi
    ret