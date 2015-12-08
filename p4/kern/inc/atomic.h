/** @file atomic.h
 *
 *  @brief This file contains headers for atomic assembly functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 */

#ifndef KERN_INC_ATOMIC_H
#define KERN_INC_ATOMIC_H

/** @brief Atomically add, then return the previous value
 *
 *  @param ptr The integer to atomically increment
 *  @param val The amount to add
 *  @return The pre-increment value in ptr
 **/
int atomic_xadd(volatile int* ptr, int val);

#endif /* KERN_INC_ATOMIC_H */
