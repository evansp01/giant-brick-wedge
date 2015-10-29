/** @file kernel_test.h
 *  @brief Interface kernel test functions on startup
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERNEL_TEST_H_
#define KERNEL_TEST_H_

void test_process_vm();
void vm_diagnose(void* page_directory);

#endif // KERNEL_TEST_H_