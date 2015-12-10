/** @file thr_internals.h
 *
 *  @brief This file contains headers for things internal to libthread
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <atomic.h>

//exit.c headers
void threaded_exit();

//malloc.c headers
void initialize_malloc();

//frame_alloc.c headers
int frame_alloc_init(unsigned int size, void* stack_high, void* stack_low);
void* alloc_frame();
void free_frame(void* frame);
void free_frame_and_vanish(void* frame);

enum stack_status {
    NOT_ON_STACK,
    UNALLOCATED_PAGE,
    FIRST_STACK,
    THREAD_STACK
};
enum stack_status get_address_stack(void *addr, void** stack);

//thread.c headers
void ensure_tcb_exists(void* stack, int tid);

/* thr_create.S headers */

/** @brief Create a thread
 *  @return The thread id of the spawned thread
 **/
int thr_create();

/** @brief Free the current stack frame, and vanish the thread
 *
 *  @param free A pointer to the free flag for the current frame
 *  @return void
 **/
void free_and_vanish(volatile int* free);

/** @brief Get the current value of esp
 *  @return The value of esp
 **/
void* get_esp();

#endif /* THR_INTERNALS_H */
