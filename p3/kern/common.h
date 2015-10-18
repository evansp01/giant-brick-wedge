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

//creating page directories
page_directory_t *init_kernel_vm();
page_directory_t *init_process_vm();

//writing to vm from kernel
int vm_copy_file(void *address, char* filename, int offset, int size);
int vm_copy_buf(void *address, void *buffer, int size);
int vm_zero(void *address, int size);


#endif // COMMON_H_
