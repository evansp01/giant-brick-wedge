.global sleep
sleep:
    pushl %esi
    movl 8(%esp), %esi
    int $0x4B
    popl %esi
    ret