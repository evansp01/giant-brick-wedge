/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */



#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

void frame_alloc_init(unsigned int size, void *stack_high, void *stack_low);
void* alloc_frame();
void free_frame(void* frame);
void free_frame_and_vanish(void* frame);

//assembly function frees page and vanishes
void free_and_vanish(int* free_attempted);
void *get_esp();

//atomics
int atomic_xchg(volatile int *ptr, int value);
int atomic_cas(volatile int *ptr, int newval, int oldval);
void atomic_inc(volatile int *ptr);
void atomic_dec(volatile int *ptr);


#endif /* THR_INTERNALS_H */
