/** @file thr_internals.h
 *
 *  @brief This file contains headers for things internal to libthread
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
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

/* thr_create.S headers */

/** @brief Create a thread
 *  @return The thread id of the spawned thread
 **/
int thr_create();

/** @brief Free the current stack frame, and vanish the thread
 *
 *  @param free_attempted A pointer to the free flag for the current frame
 *  @return void
 **/
void free_and_vanish(volatile int* free);

/** @brief Get the current value of esp
 *  @return The value of esp
 **/
void* get_esp();

/* atomic.S headers */

/** @brief Atomically exchange two integers
 *
 *  @param ptr The memory location to exchange
 *  @param value The value to exchange
 *  @return The value found at ptr
 **/
int atomic_xchg(volatile int* ptr, int value);

/** @brief Atomically compare and swap two values if they have not changed
 *
 *  @param ptr A pointer to the memory address to swap
 *  @param newval The new value to set *ptr to
 *  @param oldval The expected value of *ptr
 *  @return True if the swap was performed
 **/
int atomic_cas(volatile int* ptr, int newval, int oldval);

/** @brief Atomicallty increment an integer
 *
 *  @param ptr The integer to atomically increment
 **/
void atomic_inc(volatile int* ptr);

/** @brief Atomicallty increment an integer
 *
 *  @param ptr The integer to atomically increment
 **/
void atomic_dec(volatile int* ptr);

#endif /* THR_INTERNALS_H */
