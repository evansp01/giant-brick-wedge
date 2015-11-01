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


