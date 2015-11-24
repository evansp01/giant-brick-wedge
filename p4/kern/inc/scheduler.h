/** @file scheduler.h
 *  @brief Interface for scheduler functions
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 *  @bug No known bugs
 **/

#ifndef KERN_INC_SCHEDULER_H
#define KERN_INC_SCHEDULER_H

#include <stdint.h>
#include <control_block.h>

uint32_t get_ticks();
int yield(int yield_tid);
void init_scheduler(tcb_t *idle, tcb_t *first);
void run_scheduler(uint32_t ticks);

void schedule(tcb_t* tcb, thread_state_t expected);
void schedule_interrupts_disabled(tcb_t* tcb, thread_state_t expected);
int user_schedule(tcb_t *tcb, mutex_t *mp);

void kill_thread(tcb_t* tcb);
void deschedule(tcb_t* tcb, thread_state_t new_state);
void deschedule_and_drop(tcb_t* tcb, mutex_t* mp, thread_state_t new_state);
int user_deschedule(tcb_t* tcb, uint32_t esi);

int add_sleeper(tcb_t* tcb, uint32_t ticks);
void release_sleeper(tcb_t *tcb);

#endif // KERN_INC_SCHEDULER_H
