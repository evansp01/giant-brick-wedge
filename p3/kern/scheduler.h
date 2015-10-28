/** @file scheduler.h
 *  @brief Interface for scheduler functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <datastructures/variable_queue.h>

/** @brief Structure for a list of threads */
Q_NEW_HEAD(runnable_queue_t, tcb);

void init_scheduler();
void run_next();
void schedule(tcb_t *tcb);

#endif // SCHEDULER_H_