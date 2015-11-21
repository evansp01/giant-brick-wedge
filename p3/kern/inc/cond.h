/** @file cond.h
 *  @brief Interface for condition variables
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_COND_H
#define KERN_INC_COND_H

#include <mutex.h>
#include <variable_queue.h>

/** @brief Structure for a list of threads */
Q_NEW_HEAD(tcb_list_cond_t, tcb);

/** @brief Condition variable states */
typedef enum {
    DESTROYED,
    INITIALIZED
} cond_state_t;

/** @brief The structure for condition variables */
typedef struct cond {
    tcb_list_cond_t waiting;
    cond_state_t state;
} cond_t;

void cond_init( cond_t *cv );
void cond_destroy( cond_t *cv );
void cond_wait( cond_t *cv, mutex_t *mp );
void cond_signal( cond_t *cv );

#endif // KERN_INC_COND_H