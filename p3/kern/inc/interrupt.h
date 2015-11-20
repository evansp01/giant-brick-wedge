/** @file interrupt.h
 *  @brief Interface for interrupt module
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/
 
#ifndef KERN_INC_INTERRUPT_H
#define KERN_INC_INTERRUPT_H

void install_exceptions();
void install_syscalls();
void install_devices();
/** @brief Switches to user mode
 *
 *  @param esp Stack pointer with values to be restored
 *  @return Void
 **/
void go_to_user_mode(void *esp);


#endif // KERN_INC_INTERRUPT_H

