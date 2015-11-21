/** @file setup_idt.h
 *  @brief Interface for functions to set up the IDT
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef IDT_H_
#define IDT_H_

/** @brief Flags which specify that an idt entry is a trap gate */
#define TRAP 0x7
/** @brief Flags which specify that an idt entry is a interrupt gate */
#define INTERRUPT 0x6
/** @brief Flags to say idt entry's handler runs with kernel priveledge */
#define KERNEL 0
/** @brief Flags to say idt entry's handler runs with user priveledge */
#define USER 3

void set_idt_exception(void* handler, int type, int index);
void set_idt_syscall(void* handler, int index);
void set_idt_device(void* handler, int type, int index);
void set_idt(void* handler, int segment, int type, int privilege, int index);

#endif // IDT_H_
