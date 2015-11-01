/** @file mutex.h
 *  @brief Interface for locking functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef MUTEX_H_
#define MUTEX_H_

/** @brief Struct for mutexes */
typedef struct mutex {
    volatile int lock;
    volatile int waiting;
    volatile int owner;
} mutex_t;

int mutex_init(mutex_t* mp);
void mutex_destroy(mutex_t* mp);
void mutex_lock(mutex_t* mp);
void mutex_unlock(mutex_t* mp);

#endif // MUTEX_H_