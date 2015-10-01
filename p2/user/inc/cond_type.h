/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H
#include <mutex.h>
#include <array_queue.h>


TYPEDEF_QUEUE(tid_queue_t, int);

typedef struct cond {
    mutex_t m;
    tid_queue_t waiting;
} cond_t;

#endif /* _COND_TYPE_H */
