#ifndef COMMON_H_
#define COMMON_H_


#include <vm.h>
#include <datastructures/variable_queue.h>


//fault handler stuff
void handle_kernel_fault();
void handle_user_fault();
void page_nonexistant();
void page_permissions();
void page_zfod();

#endif // COMMON_H_
