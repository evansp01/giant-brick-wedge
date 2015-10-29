/** @file common.h
 *  @brief (Temporary) Interface for fileless functions
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef COMMON_H_
#define COMMON_H_

#include <vm.h>
#include <variable_queue.h>

//fault handler stuff
void handle_kernel_fault();
void handle_user_fault();
void page_nonexistant();
void page_permissions();
void page_zfod();
int initialize_devices(void (*tickback)(unsigned int));

#endif // COMMON_H_
