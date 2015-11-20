#ifndef INTERRUPT_H
#define INTERRUPT_H

void install_exceptions();
void install_syscalls();
void install_devices();
/** @brief Switches to user mode
 *
 *  @param esp Stack pointer with values to be restored
 *  @return Void
 **/
void go_to_user_mode(void *esp);


#endif

