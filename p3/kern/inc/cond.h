/** @file cond.h
 *  @brief Interface for condition variables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef COND_H_
#define COND_H_

#include <mutex.h>
#include <variable_queue.h>

/** @brief The structure for the condition variable queue node */
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

int cond_init( cond_t *cv );
void cond_destroy( cond_t *cv );
void cond_wait( cond_t *cv, mutex_t *mp );
void cond_signal( cond_t *cv );
void cond_broadcast( cond_t *cv );

#endif // COND_H_