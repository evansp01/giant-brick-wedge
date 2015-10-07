/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

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

//thr_create.S headers
void free_and_vanish(volatile int* free_attempted);
void* get_esp();

//atomic.S headers
int atomic_xchg(volatile int* ptr, int value);
int atomic_cas(volatile int* ptr, int newval, int oldval);
void atomic_inc(volatile int* ptr);
void atomic_dec(volatile int* ptr);

#endif /* THR_INTERNALS_H */
