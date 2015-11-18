/** @file scheduler.h
 *  @brief Interface for scheduler functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>
#include <control.h>

uint32_t get_ticks();
int yield(int yield_tid);
void init_scheduler(tcb_t *idle, tcb_t *first);
void run_scheduler(uint32_t ticks);
int schedule(tcb_t *tcb);
int schedule_interrupts_disabled(tcb_t* tcb);
void kill_thread(tcb_t* tcb, ppd_t* ppd);
void deschedule(tcb_t *tcb);
int user_deschedule(tcb_t* tcb, uint32_t esi);
void deschedule_and_drop(tcb_t *tcb, mutex_t *mp);
void scheduler_pre_switch(tcb_t* from, tcb_t* to);

int add_sleeper(tcb_t* tcb, uint32_t ticks);
void release_sleeper(tcb_t *tcb);

#endif // SCHEDULER_H_
