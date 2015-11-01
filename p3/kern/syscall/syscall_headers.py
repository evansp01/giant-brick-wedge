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
    print("/** @brief The {} syscall".format(call))
    print(" *  @param state The current state in user mode")
    print(" *  @return void")
    print(" */")
    print("void {}_syscall(ureg_t state)".format(call))
    print("{")
    print("    tcb_t* tcb = get_tcb();")
    print('    lprintf("Thread %d called {}. Not yet implemented", tcb->id);'.format(call))
    print("    while(1) {")
    print("        continue;")
    print("    }")
    print("}")
    print("")
