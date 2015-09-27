.global set_cursor_pos
set_cursor_pos:
    pushl %esi
    leal 8(%esp), %esi
    int $0x50
    popl %esi
    ret