/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H
#include <mutex.h>
#include <variable_queue.h>

typedef struct node {
    Q_NEW_LINK(node) node_link;
    int tid;
    int reject;
} node_t;

/** @brief A queue of waiting thread ids for condition variables */
Q_NEW_HEAD(tid_list_t, node);

/** @brief The structure for condition variables */
typedef struct cond {
    mutex_t m;
    tid_list_t waiting;
} cond_t;

#endif /* _COND_TYPE_H */
