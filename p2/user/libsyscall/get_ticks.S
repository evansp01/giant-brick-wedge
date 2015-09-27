.global get_ticks
get_ticks:
    int $0x53
    ret