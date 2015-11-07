/** @file sem.h
 *  @brief Interface for semaphores
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SEM_H_
#define SEM_H_

#include <cond.h>
#include <mutex.h>

/** @brief Struct for semaphores
 */
typedef struct sem {
    mutex_t m;
    cond_t cv;
    volatile int count;
} sem_t;

int sem_init( sem_t *sem, int count );
void sem_wait( sem_t *sem );
void sem_signal( sem_t *sem );
void sem_destroy( sem_t *sem );

#endif // COND_H_