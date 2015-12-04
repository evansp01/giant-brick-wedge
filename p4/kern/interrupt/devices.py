#!/usr/bin/env python3


things = range(32,256)


for i in things:
    print("EXCEPTION_ASM_WRAPPER_DEVICE {}".format(i))

print()
print()
print()

for i in things:
    print()
    print("/** @brief Wrapper for user interrupt {}".format(i))
    print(" *  @return void")
    print(" **/")
    print("INT_ASM_H({});".format(i))


print()
print()
print()

print("switch (INTERRUPT_TO_INSTALL) {")
for i in things:
    print("case {}:".format(i))
    print("    set_idt_device(INT_ASM({}), {});".format(i, i))
    print("    break;")
print("}")
