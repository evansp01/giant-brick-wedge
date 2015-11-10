/** @file setup_idt.h
 *  @brief Interface for functions to set up the IDT
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef IDT_H_
#define IDT_H_

#include <ureg.h>

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
void init_syscall_sem();

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);
typedef struct swexn {
    swexn_handler_t handler;
    void *arg;
    void *stack;
} swexn_t;
typedef struct swexn_stack {
    void *ret_addr;
    void *arg;
    void *ureg;
    ureg_t state;
} swexn_stack_t;

void register_swexn(swexn_handler_t handler, void *arg, void *stack);
void deregister_swexn();

#endif // IDT_H_
