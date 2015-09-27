.global set_term_color
set_term_color:
    pushl %esi
    movl 8(%esp), %esi
    int $0x4F
    popl %esi
    ret