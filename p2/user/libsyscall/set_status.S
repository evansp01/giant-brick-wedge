

.global set_status
set_status:
    pushl %esi
    movl 8(%esp), %esi
    int $0x59
    popl %esi
    ret
