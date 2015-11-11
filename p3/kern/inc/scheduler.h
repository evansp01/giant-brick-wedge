/** @file scheduler.h
 *  @brief Interface for scheduler functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_
#include <stdint.h>


uint32_t get_ticks();
int yield(int yield_tid);
void init_scheduler(tcb_t *idle, tcb_t *first);
void run_scheduler(uint32_t ticks);
int schedule(tcb_t *tcb);
void kill_thread(tcb_t* tcb, pcb_t *pcb);
void deschedule(tcb_t *tcb);
int user_deschedule(tcb_t* tcb, uint32_t esi);
void deschedule_and_drop(tcb_t *tcb, mutex_t *mp);
void scheduler_post_switch();
void scheduler_pre_switch(tcb_t* from, tcb_t* to);


int init_sleep();
uint32_t add_sleeper(tcb_t* tcb, uint32_t ticks);
void schedule_sleepers(uint32_t current);
void release_sleeper(uint32_t slept);

#endif // SCHEDULER_H_
