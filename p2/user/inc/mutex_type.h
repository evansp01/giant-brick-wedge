/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {
    volatile int lock;
    volatile int waiting;
    volatile int owner;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
