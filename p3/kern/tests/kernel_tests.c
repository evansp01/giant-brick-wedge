/** @file kernel_tests.c
 *  @brief Functions to test kernel initialization
 *
 *  @author Jonathan Ong (jonathao)
 *  @author Evan Palmer (esp)
 *  @bug No known bugs.
 **/
 
#include <kernel_tests.h>
#include <vm.h>
#include <simics.h>
#include <malloc.h>

#define PAGE_SIZE_SQUARED PAGE_SIZE* PAGE_SIZE
#define NUM_INTEGERS 1345

/** @brief Tests to check that VM initialized correctly
 *
 *  @return void
 **/
void test_process_vm()
{
    int i, j;
    char* memory = 0;
    page_directory_t* dir = create_page_directory();

    int* reference = malloc(NUM_INTEGERS * sizeof(int));
    int* copy = malloc(NUM_INTEGERS * sizeof(int));

    //set up reference with some values
    for (i = 0; i < NUM_INTEGERS; i++) {
        reference[i] = i + 1;
        copy[i] = 0;
    }

    for (i = 0; i < PAGE_SIZE_SQUARED * 10; i += PAGE_SIZE_SQUARED - 200) {
        void* virtual = &memory[i];
        vm_write(dir, virtual, reference, NUM_INTEGERS * sizeof(int));
        vm_read(dir, virtual, copy, NUM_INTEGERS * sizeof(int));

        //check to see read and write worked
        int failed = 0;
        for (j = 0; j < NUM_INTEGERS; j++) {
            if (copy[j] != reference[j]) {
                lprintf("read write error at %d %d vd %d", j, copy[j],
                        reference[j]);
                failed = 1;
                break;
            }
        }
        if (!failed) {
            lprintf("passed test writing to %x", i);
        }

        //reset copy
        for (j = 0; j < NUM_INTEGERS; j++) {
            copy[j] = 0;
        }
    }
}

/** @brief Looks up some kinda random addresses to see if the identity map works
 *  @param page_directory The kernel page directory
 *  @return void
 **/
void vm_diagnose(void* page_directory)
{
    char* memory = 0;
    int i;
    for (i = 0; i < PAGE_SIZE_SQUARED * 12; i += PAGE_SIZE_SQUARED - 200) {
        void* physical;
        void* virtual = &memory[i];
        if (vm_to_physical(page_directory, virtual, &physical, NULL) < 0) {
            lprintf("Error could not find mapping");
            continue;
        }
        if (virtual != physical) {
            lprintf("Error %x maps to %x", (unsigned int)virtual,
                    (unsigned int)physical);
        }
    }
}