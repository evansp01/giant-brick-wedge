/** @file mutex.h
 *  @brief Interface for locking functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_MUTEX_H
#define KERN_INC_MUTEX_H

#include <variable_queue.h>

/** @brief Structure for a list of threads */
Q_NEW_HEAD(tcb_list_t, tcb);

/** @brief Struct for mutexes */
typedef struct mutex {
    volatile int owner;
    volatile int count;
    tcb_list_t waiting;
} mutex_t;

void lock();
void unlock();
void enable_mutexes();
void mutex_init(mutex_t* mp);
void mutex_destroy(mutex_t* mp);
void mutex_lock(mutex_t* mp);
void mutex_unlock(mutex_t* mp);
void scheduler_mutex_unlock(mutex_t* mp);

#endif // KERN_INC_MUTEX_H
