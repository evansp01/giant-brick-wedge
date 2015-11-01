#!/usr/bin/env python3

syscalls = [
    "fork",
    "exec",
    "set_status",
    "vanish",
    "task_vanish",
    "wait",
    "gettid",
    "yield",
    "deschedule",
    "make_runnable",
    "get_ticks",
    "sleep",
    "new_pages",
    "remove_pages",
    "getchar",
    "readline",
    "print",
    "set_term_color",
    "set_cursor_pos",
    "get_cursor_pos",
    "halt",
    "readfile",
    "misbehave",
    "swexn",
    ]

for call in syscalls:
    print("/** @brief Wrapper for {} syscall handler".format(call))
    print(" *  @return void")
    print(" */")
    print("NAME_ASM_H({}_syscall);".format(call))
    print("")
