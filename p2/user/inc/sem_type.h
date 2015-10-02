/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <cond_type.h>
#include <mutex_type.h>

typedef struct sem {
    mutex_t m;
    cond_t cv;
    volatile int count;
} sem_t;

#endif /* _SEM_TYPE_H */
