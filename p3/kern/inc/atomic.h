/** @file atomic.h
 *  @brief Interface for atomic functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef ATOMIC_H_
#define ATOMIC_H_

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

/** @brief Atomically increment an integer
 *
 *  @param ptr The integer to atomically increment
 **/
void atomic_inc(volatile int* ptr);

/** @brief Atomically increment an integer
 *
 *  @param ptr The integer to atomically increment
 **/
void atomic_dec(volatile int* ptr);

#endif // ATOMIC_H_