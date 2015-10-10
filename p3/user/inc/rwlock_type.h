/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex_type.h>
#include <cond_type.h>

#define RWLOCK_READ  0
#define RWLOCK_WRITE 1

/** @brief Struct for reader/writer locks
 */
typedef struct rwlock {
    mutex_t m;
    cond_t cv_readers;
    cond_t cv_writers;
    volatile int mode;
    volatile int owner;
    volatile int writers_waiting;
    volatile int writers_queued;
    volatile int readers_waiting;
    volatile int readers_active;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
