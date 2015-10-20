#ifndef IDT_H_
#define IDT_H_

#define TRAP 0x7
#define INTERRUPT 0x6
#define KERNEL 0
#define USER 3

void set_idt_exception(void* handler, int type, int index);
void set_idt_syscall(void* handler, int index);
void set_idt_device(void* handler, int type, int index);
void set_idt(void* handler, int segment, int type, int privilege, int index);
void install_exceptions();
void install_syscalls();
void install_devices();

#endif // IDT_H_
