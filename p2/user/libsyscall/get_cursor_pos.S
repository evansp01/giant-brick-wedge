.global get_cursor_pos
get_cursor_pos:
    pushl %esi
    leal 8(%esp), %esi
    int $0x51
    popl %esi
    ret