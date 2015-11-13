/** @file common.h
 *  @brief (Temporary) Interface for fileless functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef COMMON_H_
#define COMMON_H_

#include <control.h>

//fault handler stuff
int init_timer();
void init_syscalls();
int readline(int len, char *buf, tcb_t *tcb);

#endif // COMMON_H_
