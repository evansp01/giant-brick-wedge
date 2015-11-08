/** @file scheduler.h
 *  @brief Interface for scheduler functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <variable_queue.h>

void init_scheduler(tcb_t *idle, tcb_t *first);
void run_next();
int schedule(tcb_t *tcb);
int deschedule(tcb_t* tcb, uint32_t esi);
void deschedule_and_drop(tcb_t *tcb, mutex_t *mp);

#endif // SCHEDULER_H_
