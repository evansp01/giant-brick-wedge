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

/** @brief Structure for a list of threads */
Q_NEW_HEAD(tcb_list_cond_t, tcb);

/** @brief The structure for condition variables */
typedef struct cond {
    tcb_list_cond_t waiting;
} cond_t;

void cond_init( cond_t *cv );
void cond_destroy( cond_t *cv );
void cond_wait( cond_t *cv, mutex_t *mp );
void cond_signal( cond_t *cv );
void cond_broadcast( cond_t *cv );

#endif // COND_H_   